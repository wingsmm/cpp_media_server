#pragma once
#include "server_config.hpp"
#include "../converter/protocol_manager.hpp"
#include <atomic>
#include <memory>

struct ServerStats {
    // 连接统计
    std::atomic<uint32_t> current_connections{0};
    std::atomic<uint32_t> max_connections{0};
    std::atomic<uint64_t> total_connections{0};
    
    // 带宽统计
    std::atomic<uint64_t> bytes_in{0};
    std::atomic<uint64_t> bytes_out{0};
    
    // 性能统计
    std::atomic<uint64_t> cpu_usage{0};
    std::atomic<uint64_t> memory_usage{0};
    std::atomic<uint64_t> avg_convert_delay{0};
    
    // 错误统计
    std::atomic<uint32_t> protocol_errors{0};
    std::atomic<uint32_t> convert_errors{0};
    std::atomic<uint32_t> memory_errors{0};
};

class MediaServer {
public:
    static MediaServer* get_instance() {
        static MediaServer instance;
        return &instance;
    }

    bool init(const std::string& config_file = "");
    bool start();
    void stop();
    void run();
    
    // 配置相关
    bool reload_config();
    void dump_stats();
    const ServerStats& get_stats() const { return stats_; }

private:
    MediaServer() = default;
    ~MediaServer() = default;
    
    // 信号处理
    void setup_signal_handlers();
    void handle_signal(int signo);
    
    // 监控统计
    void update_stats();
    void log_stats();
    
    // 服务管理
    bool start_rtmp_server();
    void stop_rtmp_server();
    bool start_webrtc_server();
    void stop_all_servers();
    
    ServerConfig config_;
    std::atomic<bool> running_{false};
    ProtocolManager* protocol_mgr_{nullptr};
    ServerStats stats_;
}; 