#pragma once
#include "utils/av/media_packet.hpp"
#include <string>
#include <memory>
#include <functional>
#include "utils/data_buffer.hpp"

class WebRTCPeer {
public:
    struct Config {
        std::string stun_server;
        std::string turn_server;
        std::string turn_username;
        std::string turn_password;
    };

    WebRTCPeer(const std::string& peer_id, const Config& config);
    ~WebRTCPeer();

    bool init();
    void close();

    // 信令相关
    bool set_remote_description(const std::string& sdp);
    bool add_ice_candidate(const std::string& candidate);
    std::string create_answer();

    // 媒体相关
    bool send_media_packet(MEDIA_PACKET_PTR pkt);

    // 回调设置
    using DataCallback = std::function<void(MEDIA_PACKET_PTR)>;
    void set_data_callback(DataCallback cb) { data_callback_ = cb; }

private:
    // ICE相关处理
    void on_ice_candidate(const std::string& candidate);
    void on_ice_state_change(const std::string& state);
    
    // 媒体相关处理
    void on_rtp_packet(const uint8_t* data, size_t len);
    void on_rtcp_packet(const uint8_t* data, size_t len);

    std::string peer_id_;
    Config config_;
    bool initialized_ = false;
    DataCallback data_callback_;

    // WebRTC相关成员
    // TODO: 添加WebRTC相关的成员变量
}; 