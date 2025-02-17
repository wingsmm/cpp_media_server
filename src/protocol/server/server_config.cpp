#include "server_config.hpp"
#include "utils/logger.hpp"
#include <fstream>
#include "json.hpp"

using json = nlohmann::json;

bool ServerConfig::load_from_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        log_errorf("Failed to open config file: %s", filename.c_str());
        return false;
    }

    json root;
    try {
        root = json::parse(file);
    } catch (const json::parse_error& e) {
        log_errorf("Failed to parse config file: %s", e.what());
        return false;
    }

    // 解析服务器配置
    if (root.contains("server")) {
        const auto& server_config = root["server"];
        server.rtmp_port = server_config.value("rtmp_port", 1935);
        server.webrtc_port = server_config.value("webrtc_port", 8000);
        server.max_connections = server_config.value("max_connections", 1000);
        
        if (server_config.contains("ssl")) {
            server.ssl.cert_file = server_config["ssl"].value("cert_file", std::string());
            server.ssl.key_file = server_config["ssl"].value("key_file", std::string());
        }
    }

    // 解析媒体配置
    if (root.contains("media")) {
        const auto& media_config = root["media"];
        media.video_buffer_size = media_config.value("video_buffer_size", 1048576);
        media.audio_buffer_size = media_config.value("audio_buffer_size", 65536);
    }

    // 解析日志配置
    if (root.contains("log")) {
        const auto& log_config = root["log"];
        log.level = log_config.value("level", std::string("info"));
        log.file = log_config.value("file", std::string());
    }

    return true;
}

bool ServerConfig::save_to_file(const std::string& filename) {
    json root;
    
    // 服务器配置
    root["server"]["rtmp_port"] = server.rtmp_port;
    root["server"]["webrtc_port"] = server.webrtc_port;
    root["server"]["max_connections"] = server.max_connections;
    root["server"]["ssl"]["cert_file"] = server.ssl.cert_file;
    root["server"]["ssl"]["key_file"] = server.ssl.key_file;

    // 媒体配置
    root["media"]["video_buffer_size"] = media.video_buffer_size;
    root["media"]["audio_buffer_size"] = media.audio_buffer_size;

    // 日志配置
    root["log"]["level"] = log.level;
    root["log"]["file"] = log.file;

    // 写入文件
    std::ofstream file(filename);
    if (!file.is_open()) {
        log_errorf("Failed to open config file for writing: %s", filename.c_str());
        return false;
    }

    file << root.dump(4);
    return true;
} 