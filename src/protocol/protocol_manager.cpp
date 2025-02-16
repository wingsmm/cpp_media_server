#include "protocol_manager.hpp"
#include "rtmp2webrtc_converter.hpp"
#include "webrtc2rtmp_converter.hpp"
#include "logger.hpp"

std::shared_ptr<ProtocolConverter> ProtocolManager::create_converter(
    const std::string& from_protocol,
    const std::string& to_protocol) {

    std::shared_ptr<ProtocolConverter> converter;

    if (from_protocol == "rtmp" && to_protocol == "webrtc") {
        converter = std::make_shared<Rtmp2WebRTCConverter>();
    } else if (from_protocol == "webrtc" && to_protocol == "rtmp") {
        converter = std::make_shared<WebRTC2RtmpConverter>();
    } else {
        log_errorf("Unsupported protocol conversion: %s -> %s", 
                   from_protocol.c_str(), to_protocol.c_str());
        return nullptr;
    }

    // 创建默认配置
    ProtocolConverter::Config config;
    if (!converter->init(config)) {
        log_errorf("Failed to initialize converter: %s -> %s",
                   from_protocol.c_str(), to_protocol.c_str());
        return nullptr;
    }

    return converter;
} 