#include "protocol_manager.hpp"
#include "rtmp2webrtc_converter.hpp"
#include "webrtc2rtmp_converter.hpp"
#include "utils/logger.hpp"

std::shared_ptr<ProtocolConverter> ProtocolManager::create_converter(
    const std::string& from_protocol,
    const std::string& to_protocol) {
    
    std::shared_ptr<ProtocolConverter> converter = nullptr;
    
    // RTMP -> WebRTC
    if (from_protocol == "rtmp" && to_protocol == "webrtc") {
        converter = std::make_shared<Rtmp2WebRTCConverter>();
        log_infof("Created RTMP to WebRTC converter");
    }
    // WebRTC -> RTMP
    else if (from_protocol == "webrtc" && to_protocol == "rtmp") {
        converter = std::make_shared<WebRTC2RtmpConverter>();
        log_infof("Created WebRTC to RTMP converter");
    }
    else {
        log_errorf("Unsupported protocol conversion: %s -> %s",
                   from_protocol.c_str(), to_protocol.c_str());
        return nullptr;
    }
    
    return converter;
} 