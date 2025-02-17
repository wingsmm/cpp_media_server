#ifndef RTMP2WEBRTC_CONVERTER_HPP
#define RTMP2WEBRTC_CONVERTER_HPP

#include "protocol_converter.hpp"
#include "format/h264_header.hpp"
#include "format/audio_pub.hpp"
#include <vector>

class Rtmp2WebRTCConverter : public ProtocolConverter {
public:
    Rtmp2WebRTCConverter() = default;
    virtual ~Rtmp2WebRTCConverter() = default;

    virtual bool init(const Config& config) override;
    virtual int input_packet(MEDIA_PACKET_PTR pkt_ptr) override;
    virtual void cleanup() override;

private:
    int handle_video_packet(MEDIA_PACKET_PTR pkt_ptr);
    int handle_audio_packet(MEDIA_PACKET_PTR pkt_ptr); 

private:
    // 存储SPS/PPS等关键信息
    std::vector<uint8_t> sps_data_;
    std::vector<uint8_t> pps_data_;
    
    // 音频相关参数
    uint8_t audio_type_ = 0;
    int sample_rate_ = 0;
    uint8_t channels_ = 0;
};

#endif 