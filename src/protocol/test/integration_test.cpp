#include "../converter/protocol_manager.hpp"
#include "utils/data_buffer.hpp"
#include <thread>
#include <chrono>

// 测试完整转换流程
void test_full_conversion_flow() {
    log_infof("=== Testing Full Conversion Flow ===");
    
    auto converter = ProtocolManager::get_instance()->create_converter("rtmp", "webrtc");
    if (!converter) {
        log_errorf("Failed to create converter");
        return;
    }
    
    bool received_video = false;
    bool received_audio = false;
    int packet_count = 0;
    
    // 设置输出回调
    converter->set_output_callback([&](MEDIA_PACKET_PTR pkt) {
        if (pkt->av_type_ == MEDIA_VIDEO_TYPE) {
            received_video = true;
        } else if (pkt->av_type_ == MEDIA_AUDIO_TYPE) {
            received_audio = true;
        }
        packet_count++;
        log_infof("Received converted packet: type=%s, size=%lu",
                  avtype_tostring(pkt->av_type_).c_str(),
                  pkt->buffer_ptr_->data_len());
    });

    // 初始化转换器
    ProtocolConverter::Config config;
    config.enable_video = true;
    config.enable_audio = true;
    int ret = converter->init(config);
    if (ret != 0) {
        log_errorf("Failed to initialize converter");
        return;
    }

    // 1. 发送视频序列头
    MEDIA_PACKET_PTR video_seq = std::make_shared<MEDIA_PACKET>();
    video_seq->av_type_ = MEDIA_VIDEO_TYPE;
    video_seq->codec_type_ = MEDIA_CODEC_H264;
    video_seq->is_seq_hdr_ = true;
    video_seq->buffer_ptr_ = std::make_shared<data_buffer>();
    uint8_t sps_data[] = {0x67, 0x42, 0x00, 0x1E}; // 模拟SPS数据
    video_seq->buffer_ptr_->append_data((char*)sps_data, sizeof(sps_data));
    ret = converter->input_packet(video_seq);
    if (ret != 0) {
        log_errorf("Failed to send video sequence header");
        return;
    }
    log_infof("Video sequence header sent");

    // 2. 发送音频序列头
    MEDIA_PACKET_PTR audio_seq = std::make_shared<MEDIA_PACKET>();
    audio_seq->av_type_ = MEDIA_AUDIO_TYPE;
    audio_seq->codec_type_ = MEDIA_CODEC_AAC;
    audio_seq->is_seq_hdr_ = true;
    audio_seq->buffer_ptr_ = std::make_shared<data_buffer>();
    uint8_t aac_header[] = {0x12, 0x10}; // 模拟AAC头
    audio_seq->buffer_ptr_->append_data((char*)aac_header, sizeof(aac_header));
    ret = converter->input_packet(audio_seq);
    if (ret != 0) {
        log_errorf("Failed to send audio sequence header");
        return;
    }
    log_infof("Audio sequence header sent");

    // 3. 发送一组音视频数据
    for (int i = 0; i < 100; i++) {
        // 视频帧
        MEDIA_PACKET_PTR video_frame = std::make_shared<MEDIA_PACKET>();
        video_frame->av_type_ = MEDIA_VIDEO_TYPE;
        video_frame->codec_type_ = MEDIA_CODEC_H264;
        video_frame->is_key_frame_ = (i % 30 == 0);  // 每30帧一个关键帧
        video_frame->pts_ = i * 40;  // 40ms一帧
        video_frame->dts_ = video_frame->pts_;
        video_frame->buffer_ptr_ = std::make_shared<data_buffer>();
        std::vector<uint8_t> video_data(1024, 0);  // 模拟视频数据
        video_frame->buffer_ptr_->append_data((char*)video_data.data(), video_data.size());
        ret = converter->input_packet(video_frame);
        if (ret != 0) {
            log_errorf("Failed to send video frame %d", i);
            return;
        }

        // 音频帧
        MEDIA_PACKET_PTR audio_frame = std::make_shared<MEDIA_PACKET>();
        audio_frame->av_type_ = MEDIA_AUDIO_TYPE;
        audio_frame->codec_type_ = MEDIA_CODEC_AAC;
        audio_frame->pts_ = i * 40;
        audio_frame->dts_ = audio_frame->pts_;
        audio_frame->buffer_ptr_ = std::make_shared<data_buffer>();
        std::vector<uint8_t> audio_data(128, 0);  // 模拟音频数据
        audio_frame->buffer_ptr_->append_data((char*)audio_data.data(), audio_data.size());
        ret = converter->input_packet(audio_frame);
        if (ret != 0) {
            log_errorf("Failed to send audio frame %d", i);
            return;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(40));  // 模拟实时速率
    }

    // 验证结果
    if (!received_video) {
        log_errorf("No video packets received");
        return;
    }
    if (!received_audio) {
        log_errorf("No audio packets received");
        return;
    }
    if (packet_count < 150) {  // 至少应该收到序列头+部分音视频帧
        log_errorf("Too few packets received: %d", packet_count);
        return;
    }
    
    log_infof("Full conversion flow test passed");
}

// 测试错误恢复流程
void test_error_recovery() {
    log_infof("=== Testing Error Recovery ===");
    
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

    // 1. 先发送一些正常数据
    for (int i = 0; i < 10; i++) {
        MEDIA_PACKET_PTR pkt = std::make_shared<MEDIA_PACKET>();
        pkt->av_type_ = MEDIA_VIDEO_TYPE;
        pkt->buffer_ptr_ = std::make_shared<data_buffer>();
        pkt->buffer_ptr_->append_data("test", 4);
        ret = converter->input_packet(pkt);
        if (ret != 0) {
            log_errorf("Failed to send normal packet %d", i);
            return;
        }
    }
    log_infof("Normal packets sent successfully");

    // 2. 发送一些异常数据
    MEDIA_PACKET_PTR invalid_pkt = std::make_shared<MEDIA_PACKET>();
    invalid_pkt->av_type_ = MEDIA_VIDEO_TYPE;
    // 不设置 buffer_ptr_，应该会失败
    ret = converter->input_packet(invalid_pkt);
    if (ret == 0) {
        log_errorf("Invalid packet should be rejected");
        return;
    }
    log_infof("Invalid packet properly rejected");

    // 3. 继续发送正常数据，验证恢复
    for (int i = 0; i < 10; i++) {
        MEDIA_PACKET_PTR pkt = std::make_shared<MEDIA_PACKET>();
        pkt->av_type_ = MEDIA_VIDEO_TYPE;
        pkt->buffer_ptr_ = std::make_shared<data_buffer>();
        pkt->buffer_ptr_->append_data("test", 4);
        ret = converter->input_packet(pkt);
        if (ret != 0) {
            log_errorf("Failed to recover after error");
            return;
        }
    }
    
    log_infof("Error recovery test passed");
}

int main() {
    log_infof("Starting integration tests...");

    test_full_conversion_flow();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    test_error_recovery();

    log_infof("Integration tests completed.");
    return 0;
} 