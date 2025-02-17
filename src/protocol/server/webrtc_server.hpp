#pragma once
#include "utils/av/media_packet.hpp"
#include <cstdint>
#include <string>
#include <memory>
#include <functional>
#include "utils/data_buffer.hpp"

class WebRTCServer {
public:
    struct Config {
        uint16_t port = 8000;
        uint32_t max_connections = 1000;
        std::string cert_file;
        std::string key_file;
    };

    WebRTCServer() = default;
    ~WebRTCServer();

    bool init(const Config& config);
    bool start();
    void stop();

    // 数据回调
    using DataCallback = std::function<void(const std::string& peer_id, MEDIA_PACKET_PTR)>;
    void set_data_callback(DataCallback cb) { data_callback_ = cb; }

private:
    void handle_websocket();
    void handle_ice_candidate(const std::string& peer_id, const std::string& candidate);
    void handle_offer(const std::string& peer_id, const std::string& sdp);
    
    Config config_;
    bool running_ = false;
    DataCallback data_callback_;
    
    // WebRTC相关成员
    // TODO: 添加WebRTC相关的成员变量
}; 