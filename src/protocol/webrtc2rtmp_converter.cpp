#include "webrtc2rtmp_converter.hpp"
#include "logger.hpp"

bool WebRTC2RtmpConverter::init(const Config& config) {
    log_infof("Initializing WebRTC to RTMP converter with config: "
              "video=%d, audio=%d, vbuf=%d, abuf=%d",
              config.enable_video, config.enable_audio,
              config.max_video_buffer_size,
              config.max_audio_buffer_size);

    config_ = config;
    state_ = STATE_INITIALIZED;
    return true;
}

int WebRTC2RtmpConverter::input_packet(MEDIA_PACKET_PTR pkt_ptr) {
    if (!pkt_ptr) {
        return -1;
    }

    if (pkt_ptr->av_type_ == MEDIA_VIDEO_TYPE) {
        return handle_video_packet(pkt_ptr);
    } else if (pkt_ptr->av_type_ == MEDIA_AUDIO_TYPE) {
        return handle_audio_packet(pkt_ptr);
    }

    return -1;
}

int WebRTC2RtmpConverter::handle_video_packet(MEDIA_PACKET_PTR pkt_ptr) {
    // TODO: 视频转换实现
    return 0;
}

int WebRTC2RtmpConverter::handle_audio_packet(MEDIA_PACKET_PTR pkt_ptr) {
    // TODO: 音频转换实现
    return 0;
}

void WebRTC2RtmpConverter::cleanup() {
    // 清理特定资源
    video_ready_ = false;
    audio_ready_ = false;

    // 调用基类清理
    ProtocolConverter::cleanup();
} 