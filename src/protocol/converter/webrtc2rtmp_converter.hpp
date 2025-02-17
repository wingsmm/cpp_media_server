#ifndef WEBRTC2RTMP_CONVERTER_HPP 
#define WEBRTC2RTMP_CONVERTER_HPP

#include "protocol_converter.hpp"

class WebRTC2RtmpConverter : public ProtocolConverter {
public:
    WebRTC2RtmpConverter() = default;
    virtual ~WebRTC2RtmpConverter() = default;

    virtual bool init(const Config& config) override;
    virtual int input_packet(MEDIA_PACKET_PTR pkt_ptr) override;
    virtual void cleanup() override;

private:
    int handle_video_packet(MEDIA_PACKET_PTR pkt_ptr);
    int handle_audio_packet(MEDIA_PACKET_PTR pkt_ptr);

private:
    // WebRTC 相关参数
    bool video_ready_ = false;
    bool audio_ready_ = false;
};

#endif 