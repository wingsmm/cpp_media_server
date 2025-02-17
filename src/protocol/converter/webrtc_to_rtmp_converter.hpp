#ifndef WEBRTC_TO_RTMP_CONVERTER_HPP
#define WEBRTC_TO_RTMP_CONVERTER_HPP

#include "protocol_converter.hpp"

class WebRTC2RtmpConverter : public ProtocolConverter {
public:
    WebRTC2RtmpConverter() = default;
    ~WebRTC2RtmpConverter() override = default;

    // 实现基类虚函数
    int init(const Config& config) override;
    int input_packet(MEDIA_PACKET_PTR pkt) override;
    void set_output_callback(const OutputCallback& cb) override;

private:
    // 处理不同类型的包
    int handle_video_packet(MEDIA_PACKET_PTR pkt);
    int handle_audio_packet(MEDIA_PACKET_PTR pkt);
    
    // 状态标志
    bool initialized_{false};
    bool has_video_sequence_{false};
    bool has_audio_sequence_{false};

    // 输出回调
    OutputCallback output_callback_;
};

#endif // WEBRTC_TO_RTMP_CONVERTER_HPP 