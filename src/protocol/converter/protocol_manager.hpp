#ifndef PROTOCOL_MANAGER_HPP
#define PROTOCOL_MANAGER_HPP

#include "protocol_converter.hpp"
#include <unordered_map>
#include <string>
#include <memory>

class ProtocolManager {
public:
    static ProtocolManager* get_instance() {
        static ProtocolManager instance;
        return &instance;
    }

    // 创建转换器
    std::shared_ptr<ProtocolConverter> create_converter(const std::string& from_protocol,
                                                      const std::string& to_protocol);

    // 获取转换器
    std::shared_ptr<ProtocolConverter> get_converter(const std::string& stream_id) {
        auto it = converters_.find(stream_id);
        if (it != converters_.end()) {
            return it->second;
        }
        return nullptr;
    }

private:
    ProtocolManager() = default;
    ProtocolManager(const ProtocolManager&) = delete;
    ProtocolManager& operator=(const ProtocolManager&) = delete;
    
    std::unordered_map<std::string, std::shared_ptr<ProtocolConverter>> converters_;
};

#endif 