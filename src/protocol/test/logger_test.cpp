#include "utils/logger.hpp"
#include <thread>
#include <chrono>

// 测试控制台输出
void test_console_output() {
    log_infof("=== Testing Console Output ===");
    
    // 设置为只输出到控制台
    Logger::get_instance()->set_filename("");
    
    log_debugf("Debug message to console");
    log_infof("Info message to console");
    log_warnf("Warning message to console");
    log_errorf("Error message to console");
    
    // 测试二进制数据输出
    uint8_t test_data[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
    log_info_data(test_data, sizeof(test_data), "Binary Data Test");
}

// 测试日志级别
void test_log_levels() {
    log_infof("=== Testing Log Levels ===");
    
    // 测试 DEBUG 级别
    Logger::get_instance()->set_level(LOGGER_DEBUG_LEVEL);
    log_debugf("This debug message should be visible");
    log_infof("This info message should be visible");
    
    // 测试 INFO 级别
    Logger::get_instance()->set_level(LOGGER_INFO_LEVEL);
    log_debugf("This debug message should NOT be visible");
    log_infof("This info message should be visible");
    
    // 测试 ERROR 级别
    Logger::get_instance()->set_level(LOGGER_ERROR_LEVEL);
    log_infof("This info message should NOT be visible");
    log_errorf("This error message should be visible");
}

// 测试实例和缓冲区
void test_instance_and_buffer() {
    log_infof("=== Testing Instance and Buffer ===");
    
    auto logger = Logger::get_instance();
    if (!logger) {
        log_errorf("Logger instance is null");
        return;
    }
    
    char* buffer = logger->get_buffer();
    if (!buffer) {
        log_errorf("Logger buffer is null");
        return;
    }
    
    log_infof("Instance and buffer test passed");
}

int main() {
    // 设置为只输出到控制台
    Logger::get_instance()->set_filename("");
    Logger::get_instance()->set_level(LOGGER_DEBUG_LEVEL);
    
    log_infof("Starting logger tests...");
    
    test_console_output();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    test_log_levels();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    test_instance_and_buffer();
    
    log_infof("Logger tests completed.");
    return 0;
} 