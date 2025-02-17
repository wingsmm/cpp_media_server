#include "media_server.hpp"
#include "utils/logger.hpp"
#include <signal.h>
#include <thread>
#include "utils/config.hpp"

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
    
    return true;
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