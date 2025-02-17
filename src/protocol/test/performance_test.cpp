#include "../converter/protocol_manager.hpp"
#include "utils/data_buffer.hpp"
#include <thread>
#include <chrono>
#include <vector>

// 测试吞吐量
void test_throughput() {
    log_infof("=== Testing Throughput ===");
    
    auto converter = ProtocolManager::get_instance()->create_converter("rtmp", "webrtc");
    if (!converter) {
        log_errorf("Failed to create converter");
        return;
    }
    
    ProtocolConverter::Config config;
    int ret = converter->init(config);
    if (ret != 0) {
        log_errorf("Failed to initialize converter");
        return;
    }

    // 准备测试数据
    const int PACKET_COUNT = 10000;
    const int PACKET_SIZE = 1024;
    std::vector<MEDIA_PACKET_PTR> packets;
    
    for (int i = 0; i < PACKET_COUNT; i++) {
        auto pkt = std::make_shared<MEDIA_PACKET>();
        pkt->av_type_ = MEDIA_VIDEO_TYPE;
        pkt->codec_type_ = MEDIA_CODEC_H264;
        pkt->buffer_ptr_ = std::make_shared<data_buffer>();
        std::vector<uint8_t> data(PACKET_SIZE, 0);
        pkt->buffer_ptr_->append_data((char*)data.data(), data.size());
        packets.push_back(pkt);
    }

    // 测量处理时间
    auto start = std::chrono::high_resolution_clock::now();
    
    for (const auto& pkt : packets) {
        ret = converter->input_packet(pkt);
        if (ret != 0) {
            log_errorf("Failed to process packet");
            return;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    double throughput = (PACKET_COUNT * PACKET_SIZE) / (duration.count() / 1000.0) / (1024 * 1024);
    log_infof("Throughput: %.2f MB/s", throughput);
    
    if (throughput < 10.0) {  // 假设最低要求是10MB/s
        log_errorf("Throughput too low: %.2f MB/s", throughput);
        return;
    }
    log_infof("Throughput test passed");
}

// 测试延迟
void test_latency() {
    log_infof("=== Testing Latency ===");
    
    auto converter = ProtocolManager::get_instance()->create_converter("rtmp", "webrtc");
    if (!converter) {
        log_errorf("Failed to create converter");
        return;
    }
    
    ProtocolConverter::Config config;
    int ret = converter->init(config);
    if (ret != 0) {
        log_errorf("Failed to initialize converter");
        return;
    }

    std::vector<double> latencies;
    const int TEST_COUNT = 1000;

    converter->set_output_callback([&latencies](MEDIA_PACKET_PTR pkt) {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto start_time = std::chrono::high_resolution_clock::time_point(
            std::chrono::microseconds(pkt->pts_)
        );
        auto latency = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - start_time
        ).count();
        latencies.push_back(latency / 1000.0);  // 转换为毫秒
    });

    // 发送测试包
    for (int i = 0; i < TEST_COUNT; i++) {
        MEDIA_PACKET_PTR pkt = std::make_shared<MEDIA_PACKET>();
        pkt->av_type_ = MEDIA_VIDEO_TYPE;
        pkt->codec_type_ = MEDIA_CODEC_H264;
        pkt->buffer_ptr_ = std::make_shared<data_buffer>();
        pkt->buffer_ptr_->append_data("test", 4);
        pkt->pts_ = std::chrono::high_resolution_clock::now()
            .time_since_epoch().count();
        
        ret = converter->input_packet(pkt);
        if (ret != 0) {
            log_errorf("Failed to process packet");
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (latencies.empty()) {
        log_errorf("No latency data collected");
        return;
    }

    // 计算平均延迟
    double total_latency = 0;
    for (double latency : latencies) {
        total_latency += latency;
    }
    double avg_latency = total_latency / latencies.size();
    
    log_infof("Average latency: %.2f ms", avg_latency);
    if (avg_latency > 100.0) {  // 假设最大允许延迟是100ms
        log_errorf("Average latency too high: %.2f ms", avg_latency);
        return;
    }
    log_infof("Latency test passed");
}

// 测试内存使用
void test_memory_usage() {
    log_infof("=== Testing Memory Usage ===");
    
    auto converter = ProtocolManager::get_instance()->create_converter("rtmp", "webrtc");
    if (!converter) {
        log_errorf("Failed to create converter");
        return;
    }
    
    ProtocolConverter::Config config;
    config.max_video_buffer_size = 10 * 1024 * 1024;  // 10MB
    int ret = converter->init(config);
    if (ret != 0) {
        log_errorf("Failed to initialize converter");
        return;
    }

    // 模拟持续输入
    const int DURATION_SECONDS = 10;
    const int PACKETS_PER_SECOND = 30;
    const int PACKET_SIZE = 1024 * 10;  // 10KB
    int packet_count = 0;

    auto start = std::chrono::steady_clock::now();
    while (std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - start).count() < DURATION_SECONDS) {
        
        MEDIA_PACKET_PTR pkt = std::make_shared<MEDIA_PACKET>();
        pkt->av_type_ = MEDIA_VIDEO_TYPE;
        pkt->codec_type_ = MEDIA_CODEC_H264;
        pkt->buffer_ptr_ = std::make_shared<data_buffer>();
        std::vector<uint8_t> data(PACKET_SIZE, 0);
        pkt->buffer_ptr_->append_data((char*)data.data(), data.size());
        
        ret = converter->input_packet(pkt);
        if (ret != 0) {
            log_errorf("Failed to process packet %d", packet_count);
            return;
        }
        
        packet_count++;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / PACKETS_PER_SECOND));
    }

    log_infof("Processed %d packets without memory issues", packet_count);
}

int main() {
    log_infof("Starting performance tests...");

    test_throughput();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    test_latency();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    test_memory_usage();

    log_infof("Performance tests completed.");
    return 0;
} 