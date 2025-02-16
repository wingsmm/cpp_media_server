#include "rtmp2webrtc_converter.hpp"
#include "logger.hpp"
#include <chrono>

bool Rtmp2WebRTCConverter::init(const Config& config) {
    if (!check_state_transition(TRANS_IDLE_TO_INIT)) {
        log_errorf("Cannot initialize from state: %d", state_);
        return false;
    }

    log_infof("Initializing RTMP to WebRTC converter with config: "
              "video=%d, audio=%d, vbuf=%d, abuf=%d",
              config.enable_video, config.enable_audio,
              config.max_video_buffer_size,
              config.max_audio_buffer_size);

    if (!config.enable_video && !config.enable_audio) {
        log_errorf("Invalid config: both video and audio are disabled");
        set_error(ERROR_CONFIG_INVALID);
        return false;
    }

    config_ = config;
    return transition_state(TRANS_IDLE_TO_INIT);
}

int Rtmp2WebRTCConverter::input_packet(MEDIA_PACKET_PTR pkt_ptr) {
    auto start_time = std::chrono::high_resolution_clock::now();

    if (!check_state_transition(TRANS_INIT_TO_RUNNING)) {
        log_errorf("Invalid state for packet input: %d", state_);
        return ERROR_STATE_INVALID;
    }

    if (!pkt_ptr || !pkt_ptr->buffer_ptr_) {
        log_errorf("Invalid packet input");
        return ERROR_INVALID_PARAM;
    }

    if (pkt_ptr->buffer_ptr_->data_len() == 0) {
        log_errorf("Empty packet buffer");
        return ERROR_BUFFER_EMPTY;
    }

    // 检查编码格式
    if (pkt_ptr->av_type_ == MEDIA_VIDEO_TYPE && 
        pkt_ptr->codec_type_ != MEDIA_CODEC_H264) {
        log_errorf("Unsupported video codec: %d", pkt_ptr->codec_type_);
        return ERROR_CODEC_NOT_SUPPORTED;
    }

    // 检查缓冲区大小
    if (pkt_ptr->av_type_ == MEDIA_VIDEO_TYPE &&
        pkt_ptr->buffer_ptr_->data_len() > config_.max_video_buffer_size) {
        log_errorf("Video packet too large: %lu > %d",
                   pkt_ptr->buffer_ptr_->data_len(),
                   config_.max_video_buffer_size);
        return ERROR_PACKET_TOO_LARGE;
    }

    // 更新资源使用统计
    update_resource_usage(pkt_ptr);

    transition_state(TRANS_INIT_TO_RUNNING);

    auto end_time = std::chrono::high_resolution_clock::now();
    auto process_time = std::chrono::duration_cast<std::chrono::microseconds>(
        end_time - start_time).count();
    
    update_statistics(pkt_ptr, process_time);
    return ERROR_OK;
}

int Rtmp2WebRTCConverter::handle_video_packet(MEDIA_PACKET_PTR pkt_ptr) {
    // TODO: 视频转换实现
    return 0;
}

int Rtmp2WebRTCConverter::handle_audio_packet(MEDIA_PACKET_PTR pkt_ptr) {
    // TODO: 音频转换实现
    return 0;
}

void Rtmp2WebRTCConverter::cleanup() {
    // 清理特定资源
    sps_data_.clear();
    pps_data_.clear();
    audio_type_ = 0;
    sample_rate_ = 0;
    channels_ = 0;

    // 调用基类清理
    ProtocolConverter::cleanup();
} 