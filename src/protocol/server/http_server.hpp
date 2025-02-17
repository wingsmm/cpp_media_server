#pragma once
#include <string>
#include <memory>
#include <functional>
#include "utils/av/media_packet.hpp"

class HttpSession;

class HttpServer {
public:
    struct Config {
        uint16_t port = 8000;
        std::string ip = "0.0.0.0";
        int timeout_seconds = 30;
    };

    HttpServer() = default;
    ~HttpServer();

    bool init(const Config& config);
    bool start();
    void stop();

    // 设置回调处理不同的HTTP请求
    using PlayCallback = std::function<void(const std::string& stream_id, 
                                          const std::string& offer,
                                          std::function<void(bool error, const std::string& answer)>)>;
    using StreamsCallback = std::function<void(std::function<void(const std::string& streams_json)>)>;
    using AddStreamCallback = std::function<void(const std::string& url, 
                                               std::function<void(bool error, const std::string& stream_id)>)>;
    using DeleteStreamCallback = std::function<void(const std::string& stream_id,
                                                  std::function<void(bool error)>)>;
    using PublishCallback = std::function<void(const std::string& stream_id,
                                         const std::string& offer,
                                         std::function<void(bool error, const std::string& answer)>)>;
    using StatsCallback = std::function<void(std::function<void(const std::string& stats_json)>)>;

    void set_play_callback(PlayCallback cb) { play_callback_ = cb; }
    void set_streams_callback(StreamsCallback cb) { streams_callback_ = cb; }
    void set_add_stream_callback(AddStreamCallback cb) { add_stream_callback_ = cb; }
    void set_delete_stream_callback(DeleteStreamCallback cb) { delete_stream_callback_ = cb; }
    void set_publish_callback(PublishCallback cb) { publish_callback_ = cb; }
    void set_stats_callback(StatsCallback cb) { stats_callback_ = cb; }

private:
    void do_accept();
    void on_accept(int client_fd);

private:
    Config config_;
    bool running_ = false;
    int server_fd_ = -1;
    
    PlayCallback play_callback_;
    StreamsCallback streams_callback_;
    AddStreamCallback add_stream_callback_;
    DeleteStreamCallback delete_stream_callback_;
    PublishCallback publish_callback_;
    StatsCallback stats_callback_;

    friend class HttpSession;
}; 