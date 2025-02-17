#include "../converter/protocol_manager.hpp"
#include "utils/data_buffer.hpp"
#include <thread>
#include <chrono>

// 测试未初始化状态
void test_uninitialized_state() {
    log_infof("=== Testing Uninitialized State ===");
    
    auto converter = ProtocolManager::get_instance()->create_converter("rtmp", "webrtc");
    if (!converter) {
        log_errorf("Failed to create converter");
        return;
    }

    // 未初始化就发送数据包
    MEDIA_PACKET_PTR pkt = std::make_shared<MEDIA_PACKET>();
    int ret = converter->input_packet(pkt);
    if (ret == 0) {
        log_errorf("Uninitialized converter should reject packets");
        return;
    }
    
    log_infof("Uninitialized state handling successful");
}

// 测试无效参数
void test_invalid_parameters() {
    log_infof("=== Testing Invalid Parameters ===");
    
    auto converter = ProtocolManager::get_instance()->create_converter("rtmp", "webrtc");
    if (!converter) {
        log_errorf("Failed to create converter");
        return;
    }

    // 测试空包
    int ret = converter->input_packet(nullptr);
    if (ret == 0) {
        log_errorf("Converter should reject null packets");
        return;
    }

    // 测试无效的协议类型
    auto invalid_converter = ProtocolManager::get_instance()->create_converter("invalid", "webrtc");
    if (invalid_converter) {
        log_errorf("Should not create converter with invalid protocol");
        return;
    }
    
    log_infof("Invalid parameters handling successful");
}

// 测试缓冲区溢出
void test_buffer_overflow() {
    log_infof("=== Testing Buffer Overflow ===");
    
    auto converter = ProtocolManager::get_instance()->create_converter("rtmp", "webrtc");
    if (!converter) {
        log_errorf("Failed to create converter");
        return;
    }

    // 设置小的缓冲区大小
    ProtocolConverter::Config config;
    config.enable_video = true;
    config.max_video_buffer_size = 1024;  // 1KB
    int ret = converter->init(config);
    if (ret != 0) {
        log_errorf("Failed to initialize converter");
        return;
    }

    // 创建大于缓冲区的数据包
    MEDIA_PACKET_PTR pkt = std::make_shared<MEDIA_PACKET>();
    pkt->av_type_ = MEDIA_VIDEO_TYPE;
    pkt->buffer_ptr_ = std::make_shared<data_buffer>();
    std::vector<uint8_t> large_data(2048, 0);  // 2KB 数据
    pkt->buffer_ptr_->append_data((char*)large_data.data(), large_data.size());
    
    ret = converter->input_packet(pkt);
    if (ret == 0) {
        log_errorf("Converter should reject oversized packets");
        return;
    }
    
    log_infof("Buffer overflow handling successful");
}

int main() {
    log_infof("Starting error handling tests...");

    test_uninitialized_state();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    test_invalid_parameters();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    test_buffer_overflow();

    log_infof("Error handling tests completed.");
    return 0;
} 