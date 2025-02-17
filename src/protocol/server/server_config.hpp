#pragma once
#include <string>
#include <cstdint>

struct ServerConfig {
    struct {
        uint16_t rtmp_port = 1935;
        uint16_t webrtc_port = 8000;
        uint32_t max_connections = 1000;
        struct {
            std::string cert_file;
            std::string key_file;
        } ssl;
    } server;

    struct {
        size_t video_buffer_size = 1048576;
        size_t audio_buffer_size = 65536;
    } media;

    struct {
        std::string level = "info";
        std::string file;
    } log;

    bool load_from_file(const std::string& filename);
    bool save_to_file(const std::string& filename);
}; 