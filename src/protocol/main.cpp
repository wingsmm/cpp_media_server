#include "server/media_server.hpp"
#include "converter/protocol_manager.hpp"
#include "utils/logger.hpp"
#include <getopt.h>

void print_usage() {
    printf("Usage: media_server [options]\n");
    printf("Options:\n");
    printf("  -c, --config <file>    Configuration file\n");
    printf("  -h, --help             Show this help\n");
}

int main(int argc, char* argv[]) {
    // 初始化日志
    Logger::get_instance()->set_level(LOGGER_INFO_LEVEL);
    
    // 初始化协议管理器
    ProtocolManager::get_instance();
    
    // 创建并初始化服务器
    auto server = MediaServer::get_instance();
    if (!server->init()) {
        log_errorf("Failed to initialize server");
        return -1;
    }
    
    // 启动服务
    if (!server->start()) {
        log_errorf("Failed to start server");
        return -1;
    }
    
    // 运行服务器主循环
    server->run();
    
    return 0;
} 