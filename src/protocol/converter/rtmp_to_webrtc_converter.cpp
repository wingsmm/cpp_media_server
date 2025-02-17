#include "rtmp_to_webrtc_converter.hpp"
#include "utils/logger.hpp"

int Rtmp2WebRTCConverter::init(const Config& config) {
    if (initialized_) {
        log_warnf("Converter already initialized");
        return -1;
    }

    // TODO: 初始化编码器和其他资源

    initialized_ = true;
    return 0;
}

int Rtmp2WebRTCConverter::input_packet(MEDIA_PACKET_PTR pkt) {
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

void Rtmp2WebRTCConverter::set_output_callback(const OutputCallback& cb) {
    output_callback_ = cb;
}

int Rtmp2WebRTCConverter::handle_video_packet(MEDIA_PACKET_PTR pkt) {
    // TODO: 处理视频包
    // 1. 解析 RTMP 视频包格式
    // 2. 提取 H264 NALU
    // 3. 转换为 RTP 包
    // 4. 通过回调发送
    return 0;
}

int Rtmp2WebRTCConverter::handle_audio_packet(MEDIA_PACKET_PTR pkt) {
    // TODO: 处理音频包
    // 1. 解析 RTMP 音频包格式
    // 2. 提取 AAC 数据
    // 3. 转换为 RTP 包
    // 4. 通过回调发送
    return 0;
} 