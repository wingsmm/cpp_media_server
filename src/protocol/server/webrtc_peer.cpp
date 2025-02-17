#include "webrtc_peer.hpp"
#include "utils/logger.hpp"

WebRTCPeer::WebRTCPeer(const std::string& peer_id, const Config& config)
    : peer_id_(peer_id), config_(config) {
}

WebRTCPeer::~WebRTCPeer() {
    close();
}

bool WebRTCPeer::init() {
    if (initialized_) {
        log_warnf("WebRTCPeer already initialized");
        return false;
    }

    // TODO: 初始化WebRTC连接
    // 1. 创建PeerConnection
    // 2. 设置ICE服务器
    // 3. 创建媒体轨道
    // 4. 设置回调

    initialized_ = true;
    return true;
}

void WebRTCPeer::close() {
    if (!initialized_) {
        return;
    }

    // TODO: 关闭WebRTC连接
    initialized_ = false;
}

bool WebRTCPeer::set_remote_description(const std::string& sdp) {
    if (!initialized_) {
        log_errorf("WebRTCPeer not initialized");
        return false;
    }

    // TODO: 设置远端SDP
    log_infof("Setting remote description for peer: %s", peer_id_.c_str());
    return true;
}

bool WebRTCPeer::add_ice_candidate(const std::string& candidate) {
    if (!initialized_) {
        log_errorf("WebRTCPeer not initialized");
        return false;
    }

    // TODO: 添加ICE候选者
    log_infof("Adding ICE candidate for peer: %s", peer_id_.c_str());
    return true;
}

std::string WebRTCPeer::create_answer() {
    if (!initialized_) {
        log_errorf("WebRTCPeer not initialized");
        return "";
    }

    // TODO: 创建Answer
    log_infof("Creating answer for peer: %s", peer_id_.c_str());
    return ""; // 返回SDP
}

bool WebRTCPeer::send_media_packet(MEDIA_PACKET_PTR pkt) {
    if (!initialized_) {
        log_errorf("WebRTCPeer not initialized");
        return false;
    }

    // TODO: 发送媒体数据
    return true;
} 