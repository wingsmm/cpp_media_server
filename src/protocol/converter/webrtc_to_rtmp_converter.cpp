#include "webrtc_to_rtmp_converter.hpp"
#include "utils/logger.hpp"

int WebRTC2RtmpConverter::init(const Config& config) {
    if (initialized_) {
        log_warnf("Converter already initialized");
        return -1;
    }

    // TODO: 初始化解码器和其他资源

    initialized_ = true;
    return 0;
}

int WebRTC2RtmpConverter::input_packet(MEDIA_PACKET_PTR pkt) {
    if (!initialized_) {
        log_errorf("Converter not initialized");
        return -1;
    }

    if (!pkt || !pkt->buffer_ptr_) {
        log_errorf("Invalid packet");
        return -1;
    }

    // 根据包类型分别处理
    if (pkt->av_type_ == MEDIA_VIDEO_TYPE) {
        return handle_video_packet(pkt);
    } else if (pkt->av_type_ == MEDIA_AUDIO_TYPE) {
        return handle_audio_packet(pkt);
    }

    log_warnf("Unknown packet type: %d", pkt->av_type_);
    return -1;
}

void WebRTC2RtmpConverter::set_output_callback(const OutputCallback& cb) {
    output_callback_ = cb;
}

int WebRTC2RtmpConverter::handle_video_packet(MEDIA_PACKET_PTR pkt) {
    // TODO: 处理视频包
    // 1. 解析 RTP 包
    // 2. 重组 H264 NALU
    // 3. 转换为 RTMP 格式
    // 4. 通过回调发送
    return 0;
}

int WebRTC2RtmpConverter::handle_audio_packet(MEDIA_PACKET_PTR pkt) {
    // TODO: 处理音频包
    // 1. 解析 RTP 包
    // 2. 重组 OPUS 数据
    // 3. 转码为 AAC
    // 4. 转换为 RTMP 格式
    // 5. 通过回调发送
    return 0;
} 