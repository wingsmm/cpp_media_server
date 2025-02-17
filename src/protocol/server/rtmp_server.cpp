#include "rtmp_server.hpp"
#include "utils/logger.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>

RtmpServer::~RtmpServer() {
    stop();
}

bool RtmpServer::init(const Config& config) {
    config_ = config;
    
    // 创建服务器socket
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        log_errorf("Failed to create socket");
        return false;
    }
    
    // 设置socket选项
    int opt = 1;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        log_errorf("Failed to set socket options");
        close(server_fd_);
        return false;
    }
    
    // 绑定地址
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(config_.port);
    
    if (bind(server_fd_, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        log_errorf("Failed to bind to port %d", config_.port);
        close(server_fd_);
        return false;
    }
    
    // 开始监听
    if (listen(server_fd_, config_.max_connections) < 0) {
        log_errorf("Failed to listen");
        close(server_fd_);
        return false;
    }
    
    return true;
}

bool RtmpServer::start() {
    if (running_) {
        log_warnf("RTMP server is already running");
        return false;
    }
    
    running_ = true;
    std::thread([this]() {
        accept_loop();
    }).detach();
    
    log_infof("RTMP server started on port %d", config_.port);
    return true;
}

void RtmpServer::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    if (server_fd_ >= 0) {
        close(server_fd_);
        server_fd_ = -1;
    }
    
    log_infof("RTMP server stopped");
}

void RtmpServer::accept_loop() {
    while (running_) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_fd_, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            if (running_) {
                log_errorf("Failed to accept client connection");
            }
            continue;
        }
        
        // 创建新线程处理客户端连接
        std::thread([this, client_fd]() {
            handle_client(client_fd);
        }).detach();
    }
}

void RtmpServer::handle_client(int client_fd) {
    // TODO: 实现RTMP客户端处理
    // 1. RTMP握手
    // 2. 接收和处理RTMP消息
    // 3. 调用data_callback_
    
    close(client_fd);
} 