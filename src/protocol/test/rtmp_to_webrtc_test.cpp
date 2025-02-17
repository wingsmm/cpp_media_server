#include "../converter/protocol_manager.hpp"
#include "utils/data_buffer.hpp"
#include <thread>
#include <chrono>

// 测试转换器创建
void test_converter_creation() {
    log_infof("=== Testing Converter Creation ===");
    
    auto converter = ProtocolManager::get_instance()->create_converter("rtmp", "webrtc");
    if (!converter) {
        log_errorf("Failed to create protocol converter");
        return;
    }
    
    log_infof("Converter creation successful");
}

// 测试视频包转换
void test_video_conversion() {
    log_infof("=== Testing Video Packet Conversion ===");
    
    auto converter = ProtocolManager::get_instance()->create_converter("rtmp", "webrtc");
    if (!converter) {
        log_errorf("Failed to create converter");
        return;
    }

    bool callback_called = false;
    converter->set_output_callback([&callback_called](MEDIA_PACKET_PTR pkt) {
        callback_called = true;
        log_infof("Converted packet - av type:%s, codec type:%s, dts:%ld, pts:%ld, len:%lu",
            avtype_tostring(pkt->av_type_).c_str(),
            codectype_tostring(pkt->codec_type_).c_str(),
            pkt->dts_,
            pkt->pts_,
            pkt->buffer_ptr_->data_len());
    });

    // 创建测试视频包
    MEDIA_PACKET_PTR video_pkt = std::make_shared<MEDIA_PACKET>();
    video_pkt->av_type_ = MEDIA_VIDEO_TYPE;
    video_pkt->codec_type_ = MEDIA_CODEC_H264;
    video_pkt->fmt_type_ = MEDIA_FORMAT_RAW;
    video_pkt->dts_ = 0;
    video_pkt->pts_ = 0;
    video_pkt->is_key_frame_ = true;
    video_pkt->is_seq_hdr_ = false;
    video_pkt->buffer_ptr_ = std::make_shared<data_buffer>();

    // 添加H264测试数据
    uint8_t test_data[] = {0x00, 0x00, 0x00, 0x01, 0x67, 0x42, 0x00, 0x1E};  // SPS NALU
    video_pkt->buffer_ptr_->append_data((char*)test_data, sizeof(test_data));

    log_infof("Sending test video packet...");
    int ret = converter->input_packet(video_pkt);
    if (ret != 0) {
        log_errorf("Video packet conversion failed with error: %d", ret);
        return;
    }
    
    // 等待回调被调用
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (!callback_called) {
        log_errorf("Output callback was not called");
        return;
    }
    
    log_infof("Video packet conversion successful");
}

// 测试配置设置
void test_converter_config() {
    log_infof("=== Testing Converter Configuration ===");
    
    auto converter = ProtocolManager::get_instance()->create_converter("rtmp", "webrtc");
    if (!converter) {
        log_errorf("Failed to create converter");
        return;
    }

    // 测试基本配置
    ProtocolConverter::Config config;
    config.enable_video = true;
    config.enable_audio = true;
    config.max_video_buffer_size = 1024 * 1024;  // 1MB

    int ret = converter->init(config);
    if (ret != 0) {
        log_errorf("Failed to initialize converter with config");
        return;
    }

    log_infof("Converter configuration successful");
}

int main() {
    log_infof("Starting RTMP to WebRTC conversion tests...");

    test_converter_creation();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    test_video_conversion();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    test_converter_config();

    log_infof("RTMP to WebRTC conversion tests completed.");
    return 0;
} 