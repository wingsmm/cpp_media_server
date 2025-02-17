#pragma once
#include "utils/av/media_packet.hpp"
#include <cstdint>
#include <string>
#include <memory>
#include <functional>
#include "utils/data_buffer.hpp"

class RtmpServer {
public:
    struct Config {
        uint16_t port = 1935;
        uint32_t max_connections = 1000;
        size_t read_buffer_size = 4096;
        size_t write_buffer_size = 4096;
    };

    RtmpServer() = default;
    ~RtmpServer();

    bool init(const Config& config);
    bool start();
    void stop();

    // 数据回调
    using DataCallback = std::function<void(const std::string& stream_id, MEDIA_PACKET_PTR)>;
    void set_data_callback(DataCallback cb) { data_callback_ = cb; }

private:
    void accept_loop();
    void handle_client(int client_fd);
    
    Config config_;
    int server_fd_ = -1;
    bool running_ = false;
    DataCallback data_callback_;
}; 