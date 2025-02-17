#include "media_server.hpp"
#include "utils/logger.hpp"
#include "json.hpp"
#include <signal.h>
#include <thread>

using json = nlohmann::json;

bool MediaServer::init(const std::string& config_file) {
    // 加载配置文件
    if (!config_file.empty()) {
        if (!config_.load_from_file(config_file)) {
            log_errorf("Failed to load config file: %s", config_file.c_str());
            return false;
        }
    }

    // 初始化日志
    // 如果配置文件中没有指定日志级别，使用默认的 INFO 级别
    LOGGER_LEVEL log_level = LOGGER_INFO_LEVEL;
    if (!config_.log.level.empty()) {
        try {
            log_level = static_cast<LOGGER_LEVEL>(std::stoi(config_.log.level));
        } catch (const std::exception& e) {
            log_warnf("Invalid log level in config, using default INFO level");
        }
    }
    Logger::get_instance()->set_level(log_level);
    
    if (!config_.log.file.empty()) {
        Logger::get_instance()->set_filename(config_.log.file);
    }

    // 初始化协议管理器
    protocol_mgr_ = ProtocolManager::get_instance();
    
    // 设置信号处理
    setup_signal_handlers();
    
    // 初始化 HTTP 服务器
    http_server_ = std::make_unique<http_server>(uv_default_loop(), config_.server.webrtc_port);
    
    // 添加 HTTP 路由处理
    http_server_->add_get_handle("/", [](const http_request* request, std::shared_ptr<http_response> response) {
        // 默认处理
        response->write("Welcome to Media Server", strlen("Welcome to Media Server"));
    });

    log_infof("HTTP server initialized on port %d", config_.server.webrtc_port);
    return true;
}

void MediaServer::handle_webrtc_play(const http_request* request, std::shared_ptr<http_response> response) {
    try {
        // 解析请求体
        std::string body(request->content_body_, request->content_length_);
        json j = json::parse(body);

        // 获取必要参数
        std::string stream_id = j["stream_id"].get<std::string>();
        std::string sdp = j["sdp"].get<std::string>();

        // 处理 WebRTC 播放请求
        // TODO: 实现具体的 WebRTC 播放逻辑

        // 返回响应
        json resp = {
            {"code", 0},
            {"msg", "success"},
            {"data", {
                {"sdp", "answer_sdp_here"}  // TODO: 替换为实际的 answer sdp
            }}
        };

        response->add_header("Access-Control-Allow-Origin", "*");
        response->add_header("Content-Type", "application/json");
        response->write(resp.dump().c_str(), resp.dump().length());
    } catch (const std::exception& e) {
        json resp = {
            {"code", -1},
            {"msg", e.what()}
        };
        response->write(resp.dump().c_str(), resp.dump().length());
    }
}

void MediaServer::handle_webrtc_publish(const http_request* request, std::shared_ptr<http_response> response) {
    try {
        // 解析请求体
        std::string body(request->content_body_, request->content_length_);
        json j = json::parse(body);

        // 获取必要参数
        std::string stream_id = j["stream_id"].get<std::string>();
        std::string sdp = j["sdp"].get<std::string>();

        // 处理 WebRTC 推流请求
        // TODO: 实现具体的 WebRTC 推流逻辑

        // 返回响应
        json resp = {
            {"code", 0},
            {"msg", "success"},
            {"data", {
                {"sdp", "answer_sdp_here"}  // TODO: 替换为实际的 answer sdp
            }}
        };

        response->add_header("Access-Control-Allow-Origin", "*");
        response->add_header("Content-Type", "application/json");
        response->write(resp.dump().c_str(), resp.dump().length());
    } catch (const std::exception& e) {
        json resp = {
            {"code", -1},
            {"msg", e.what()}
        };
        response->write(resp.dump().c_str(), resp.dump().length());
    }
}

void MediaServer::handle_webrtc_stats(const http_request* request, std::shared_ptr<http_response> response) {
    try {
        // 获取统计信息
        json stats = {
            {"connections", MediaServer::get_instance()->get_stats().current_connections.load()},
            {"bytes_in", MediaServer::get_instance()->get_stats().bytes_in.load()},
            {"bytes_out", MediaServer::get_instance()->get_stats().bytes_out.load()}
        };

        json resp = {
            {"code", 0},
            {"msg", "success"},
            {"data", stats}
        };

        response->add_header("Access-Control-Allow-Origin", "*");
        response->add_header("Content-Type", "application/json");
        response->write(resp.dump().c_str(), resp.dump().length());
    } catch (const std::exception& e) {
        json resp = {
            {"code", -1},
            {"msg", e.what()}
        };
        response->write(resp.dump().c_str(), resp.dump().length());
    }
}

bool MediaServer::start() {
    if (running_) {
        log_warnf("Server is already running");
        return false;
    }

    // 启动RTMP服务器
    if (!start_rtmp_server()) {
        log_errorf("Failed to start RTMP server");
        return false;
    }

    // 启动WebRTC服务器
    if (!start_webrtc_server()) {
        log_errorf("Failed to start WebRTC server");
        stop_rtmp_server();
        return false;
    }

    running_ = true;
    log_infof("Media server started");
    return true;
}

void MediaServer::stop() {
    if (!running_) {
        return;
    }

    running_ = false;
    stop_all_servers();
    log_infof("Media server stopped");
}

void MediaServer::run() {
    while (running_) {
        update_stats();
        log_stats();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

bool MediaServer::reload_config() {
    // TODO: 实现配置重新加载
    return true;
}

void MediaServer::dump_stats() {
    log_infof("Server Statistics:");
    log_infof("Connections: current=%u, max=%u, total=%lu",
        stats_.current_connections.load(),
        stats_.max_connections.load(),
        stats_.total_connections.load());
    log_infof("Bandwidth: in=%lu bytes, out=%lu bytes",
        stats_.bytes_in.load(),
        stats_.bytes_out.load());
    log_infof("Performance: cpu=%lu%%, memory=%lu MB, delay=%lu ms",
        stats_.cpu_usage.load(),
        stats_.memory_usage.load() / (1024 * 1024),
        stats_.avg_convert_delay.load());
    log_infof("Errors: protocol=%u, convert=%u, memory=%u",
        stats_.protocol_errors.load(),
        stats_.convert_errors.load(),
        stats_.memory_errors.load());
}

void MediaServer::setup_signal_handlers() {
    signal(SIGINT, [](int) { 
        MediaServer::get_instance()->stop();
    });
    
    signal(SIGUSR1, [](int) {
        MediaServer::get_instance()->reload_config();
    });
    
    signal(SIGUSR2, [](int) {
        MediaServer::get_instance()->dump_stats();
    });
}

void MediaServer::update_stats() {
    // TODO: 实现统计信息更新
}

void MediaServer::log_stats() {
    // TODO: 实现统计信息日志
}

bool MediaServer::start_rtmp_server() {
    // TODO: 实现RTMP服务器启动
    return true;
}

void MediaServer::stop_rtmp_server() {
    // TODO: 实现RTMP服务器停止
    log_infof("Stopping RTMP server...");
}

bool MediaServer::start_webrtc_server() {
    // TODO: 实现WebRTC服务器启动
    return true;
}

void MediaServer::stop_all_servers() {
    // TODO: 实现所有服务器停止
} 