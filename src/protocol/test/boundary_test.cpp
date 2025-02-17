#include "../converter/protocol_manager.hpp"
#include "utils/data_buffer.hpp"
#include <thread>
#include <chrono>

// 测试极限配置
void test_extreme_config() {
    log_infof("=== Testing Extreme Configurations ===");
    
    auto converter = ProtocolManager::get_instance()->create_converter("rtmp", "webrtc");
    if (!converter) {
        log_errorf("Failed to create converter");
        return;
    }

    // 测试最小配置
    ProtocolConverter::Config min_config;
    min_config.enable_video = false;
    min_config.enable_audio = false;
    min_config.max_video_buffer_size = 1024;
    int ret = converter->init(min_config);
    if (ret != 0) {
        log_errorf("Minimal config should be accepted");
        return;
    }
    log_infof("Minimal configuration test passed");

    // 测试最大配置
    auto max_converter = ProtocolManager::get_instance()->create_converter("rtmp", "webrtc");
    ProtocolConverter::Config max_config;
    max_config.enable_video = true;
    max_config.enable_audio = true;
    max_config.max_video_buffer_size = 100 * 1024 * 1024;  // 100MB
    ret = max_converter->init(max_config);
    if (ret != 0) {
        log_errorf("Maximal config should be accepted");
        return;
    }
    log_infof("Maximal configuration test passed");
}

// 测试资源限制
void test_resource_limits() {
    log_infof("=== Testing Resource Limits ===");
    
    auto converter = ProtocolManager::get_instance()->create_converter("rtmp", "webrtc");
    if (!converter) {
        log_errorf("Failed to create converter");
        return;
    }

    // 设置资源限制
    ProtocolConverter::Config config;
    config.max_video_buffer_size = 1024;  // 1KB 限制
    int ret = converter->init(config);
    if (ret != 0) {
        log_errorf("Failed to initialize converter");
        return;
    }

    // 测试大量小包
    for (int i = 0; i < 100; i++) {
        MEDIA_PACKET_PTR pkt = std::make_shared<MEDIA_PACKET>();
        pkt->av_type_ = MEDIA_VIDEO_TYPE;
        pkt->buffer_ptr_ = std::make_shared<data_buffer>();
        pkt->buffer_ptr_->append_data("test", 4);
        ret = converter->input_packet(pkt);
        if (ret != 0) {
            log_errorf("Failed to handle small packet %d", i);
            return;
        }
    }
    log_infof("Small packets test passed");

    // 测试超大包
    MEDIA_PACKET_PTR large_pkt = std::make_shared<MEDIA_PACKET>();
    large_pkt->av_type_ = MEDIA_VIDEO_TYPE;
    large_pkt->buffer_ptr_ = std::make_shared<data_buffer>();
    std::vector<uint8_t> large_data(2048, 0);  // 2KB 数据
    large_pkt->buffer_ptr_->append_data((char*)large_data.data(), large_data.size());
    
    ret = converter->input_packet(large_pkt);
    if (ret == 0) {
        log_errorf("Oversized packet should be rejected");
        return;
    }
    log_infof("Large packet test passed");
}

// 测试重复初始化
void test_repeated_init() {
    log_infof("=== Testing Repeated Initialization ===");
    
    auto converter = ProtocolManager::get_instance()->create_converter("rtmp", "webrtc");
    if (!converter) {
        log_errorf("Failed to create converter");
        return;
    }

    // 第一次初始化
    ProtocolConverter::Config config;
    int ret = converter->init(config);
    if (ret != 0) {
        log_errorf("First initialization failed");
        return;
    }
    log_infof("First initialization successful");

    // 尝试重复初始化
    ret = converter->init(config);
    if (ret == 0) {
        log_errorf("Second initialization should fail");
        return;
    }
    log_infof("Repeated initialization properly rejected");
}

int main() {
    log_infof("Starting boundary tests...");

    test_extreme_config();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    test_resource_limits();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    test_repeated_init();

    log_infof("Boundary tests completed.");
    return 0;
} 