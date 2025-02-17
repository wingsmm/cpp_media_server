#include "http_server.hpp"
#include "utils/logger.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <arpa/inet.h>
#include "json.hpp"

using json = nlohmann::json;

class HttpSession {
public:
    HttpSession(int fd, HttpServer* server) 
        : client_fd_(fd), server_(server) {}
    
    ~HttpSession() {
        if (client_fd_ >= 0) {
            close(client_fd_);
        }
    }

    void run() {
        char buffer[4096];
        int n = read(client_fd_, buffer, sizeof(buffer));
        if (n <= 0) {
            return;
        }

        // 解析HTTP请求
        std::string request(buffer, n);
        handle_request(request);
    }

private:
    void handle_request(const std::string& request) {
        try {
            // 解析请求行
            size_t pos = request.find("\r\n");
            if (pos == std::string::npos) {
                return send_error(400, "Bad Request");
            }

            std::string request_line = request.substr(0, pos);
            std::string method, path, version;
            
            // 解析方法、路径和版本
            pos = request_line.find(" ");
            if (pos == std::string::npos) {
                return send_error(400, "Bad Request");
            }
            method = request_line.substr(0, pos);
            request_line = request_line.substr(pos + 1);

            pos = request_line.find(" ");
            if (pos == std::string::npos) {
                return send_error(400, "Bad Request");
            }
            path = request_line.substr(0, pos);
            version = request_line.substr(pos + 1);

            // 处理OPTIONS请求
            if (method == "OPTIONS") {
                return send_cors_headers();
            }

            // 解析请求体
            std::string body;
            size_t body_pos = request.find("\r\n\r\n");
            if (body_pos != std::string::npos) {
                body = request.substr(body_pos + 4);
            }

            // 根据路径处理请求
            if (path == "/webrtc/play" && method == "POST") {
                handle_webrtc_play(body);
            }
            else if (path == "/webrtc/publish" && method == "POST") {
                handle_webrtc_publish(body);
            }
            else if (path == "/webrtc/stats" && method == "GET") {
                handle_webrtc_stats();
            }
            else if (path == "/streams" && method == "GET") {
                handle_get_streams();
            }
            else if (path == "/stream/add" && method == "POST") {
                handle_add_stream(body);
            }
            else if (path == "/stream/delete" && method == "DELETE") {
                handle_delete_stream(body);
            }
            else {
                send_error(404, "Not Found");
            }
        } catch (const std::exception& e) {
            log_errorf("Error handling request: %s", e.what());
            send_error(500, "Internal Server Error");
        }
    }

    void handle_webrtc_play(const std::string& body) {
        try {
            json j = json::parse(body);
            std::string stream_id = j["stream_id"].get<std::string>();
            std::string offer = j["sdp"].get<std::string>();

            server_->play_callback_(stream_id, offer, 
                [this](bool error, const std::string& answer) {
                    if (error) {
                        send_error(500, "Internal Server Error");
                        return;
                    }
                    
                    json response = {
                        {"sdp", answer}
                    };
                    send_response(200, "application/json", response.dump());
                });
        } catch (const json::exception& e) {
            send_error(400, "Invalid request body");
        }
    }

    void handle_webrtc_publish(const std::string& body) {
        try {
            json j = json::parse(body);
            std::string stream_id = j["stream_id"].get<std::string>();
            std::string offer = j["sdp"].get<std::string>();

            server_->publish_callback_(stream_id, offer, 
                [this](bool error, const std::string& answer) {
                    if (error) {
                        send_error(500, "Internal Server Error");
                        return;
                    }
                    
                    json response = {
                        {"sdp", answer}
                    };
                    send_response(200, "application/json", response.dump());
                });
        } catch (const json::exception& e) {
            send_error(400, "Invalid request body");
        }
    }

    void handle_webrtc_stats() {
        server_->stats_callback_([this](const std::string& stats_json) {
            send_response(200, "application/json", stats_json);
        });
    }

    void handle_get_streams() {
        server_->streams_callback_([this](const std::string& streams_json) {
            send_response(200, "application/json", streams_json);
        });
    }

    void handle_add_stream(const std::string& body) {
        try {
            json j = json::parse(body);
            std::string url = j["url"].get<std::string>();

            server_->add_stream_callback_(url,
                [this](bool error, const std::string& stream_id) {
                    if (error) {
                        send_error(500, "Internal Server Error");
                        return;
                    }
                    
                    json response = {
                        {"stream_id", stream_id}
                    };
                    send_response(200, "application/json", response.dump());
                });
        } catch (const json::exception& e) {
            send_error(400, "Invalid request body");
        }
    }

    void handle_delete_stream(const std::string& body) {
        try {
            json j = json::parse(body);
            std::string stream_id = j["stream_id"].get<std::string>();

            server_->delete_stream_callback_(stream_id,
                [this](bool error) {
                    if (error) {
                        send_error(500, "Internal Server Error");
                        return;
                    }
                    send_response(200, "application/json", "{}");
                });
        } catch (const json::exception& e) {
            send_error(400, "Invalid request body");
        }
    }

    void send_cors_headers() {
        std::string response = "HTTP/1.1 200 OK\r\n"
                              "Access-Control-Allow-Origin: *\r\n"
                              "Access-Control-Allow-Methods: GET, POST, DELETE, OPTIONS\r\n"
                              "Access-Control-Allow-Headers: Content-Type\r\n";
        response += "\r\n";
        send(client_fd_, response.c_str(), response.length(), 0);
    }

    void send_response(int status_code, 
                      const std::string& content_type,
                      const std::string& body) {
        std::string response = "HTTP/1.1 " + std::to_string(status_code) + " OK\r\n";
        response += "Content-Type: " + content_type + "\r\n";
        response += "Content-Length: " + std::to_string(body.length()) + "\r\n";
        response += "Access-Control-Allow-Origin: *\r\n";
        response += "Access-Control-Allow-Methods: GET, POST, DELETE, OPTIONS\r\n";
        response += "Access-Control-Allow-Headers: Content-Type\r\n";
        response += "\r\n";
        response += body;

        write(client_fd_, response.c_str(), response.length());
    }

    void send_error(int status_code, const std::string& message) {
        std::string body = "{\"error\":\"" + message + "\"}";
        send_response(status_code, "application/json", body);
    }

private:
    int client_fd_;
    HttpServer* server_;
};

bool HttpServer::init(const Config& config) {
    config_ = config;
    
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        log_errorf("Failed to create socket");
        return false;
    }

    int opt = 1;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        log_errorf("Failed to set socket options");
        close(server_fd_);
        return false;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(config_.port);

    if (bind(server_fd_, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        log_errorf("Failed to bind to port %d", config_.port);
        close(server_fd_);
        return false;
    }

    if (listen(server_fd_, SOMAXCONN) < 0) {
        log_errorf("Failed to listen");
        close(server_fd_);
        return false;
    }

    return true;
}

void HttpServer::do_accept() {
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
            on_accept(client_fd);
        }).detach();
    }
}

void HttpServer::on_accept(int client_fd) {
    HttpSession session(client_fd, this);
    session.run();
}

bool HttpServer::start() {
    if (running_) {
        return false;
    }

    running_ = true;
    
    // 启动接收线程
    std::thread([this]() {
        do_accept();
    }).detach();

    log_infof("HTTP server started on %s:%d", config_.ip.c_str(), config_.port);
    return true;
}

void HttpServer::stop() {
    if (!running_) {
        return;
    }

    running_ = false;
    
    if (server_fd_ >= 0) {
        close(server_fd_);
        server_fd_ = -1;
    }

    log_infof("HTTP server stopped");
}

HttpServer::~HttpServer() {
    stop();
} 