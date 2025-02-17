#include "../converter/protocol_manager.hpp"
#include "utils/data_buffer.hpp"
#include <thread>
#include <atomic>
#include <chrono>

// 测试多线程访问
void test_multi_thread_access() {
    log_infof("=== Testing Multi-thread Access ===");
    
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

    std::atomic<bool> stop_flag(false);
    std::vector<std::thread> threads;
    std::atomic<int> error_count(0);

    // 创建多个线程同时访问转换器
    for (int i = 0; i < 4; i++) {
        threads.emplace_back([converter, &stop_flag, &error_count]() {
            while (!stop_flag) {
                MEDIA_PACKET_PTR pkt = std::make_shared<MEDIA_PACKET>();
                pkt->av_type_ = MEDIA_VIDEO_TYPE;
                pkt->buffer_ptr_ = std::make_shared<data_buffer>();
                pkt->buffer_ptr_->append_data("test", 4);
                if (converter->input_packet(pkt) != 0) {
                    error_count++;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }

    // 运行一段时间后停止
    std::this_thread::sleep_for(std::chrono::seconds(2));
    stop_flag = true;

    for (auto& t : threads) {
        t.join();
    }

    if (error_count > 0) {
        log_errorf("Encountered %d errors during concurrent access", error_count.load());
        return;
    }
    log_infof("Multi-thread access test passed");
}

// 测试并发初始化
void test_concurrent_init() {
    log_infof("=== Testing Concurrent Initialization ===");
    
    auto converter = ProtocolManager::get_instance()->create_converter("rtmp", "webrtc");
    if (!converter) {
        log_errorf("Failed to create converter");
        return;
    }

    std::atomic<int> success_count(0);
    std::vector<std::thread> threads;

    // 多个线程同时尝试初始化
    for (int i = 0; i < 4; i++) {
        threads.emplace_back([converter, &success_count]() {
            ProtocolConverter::Config config;
            if (converter->init(config) == 0) {
                success_count++;
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    if (success_count != 1) {
        log_errorf("Expected exactly one successful initialization, got %d", success_count.load());
        return;
    }
    log_infof("Concurrent initialization test passed");
}

int main() {
    log_infof("Starting concurrency tests...");

    test_multi_thread_access();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    test_concurrent_init();

    log_infof("Concurrency tests completed.");
    return 0;
} 