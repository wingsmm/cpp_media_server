#ifndef WEBRTC_STATS_HPP
#define WEBRTC_STATS_HPP

#include <cstdint>
#include <atomic>

struct WebRTCStats {
    // RTP 统计
    std::atomic<uint64_t> rtp_packets_received{0};
    std::atomic<uint64_t> rtp_bytes_received{0};
    std::atomic<uint64_t> rtp_packets_lost{0};
    
    // 延迟统计
    std::atomic<uint64_t> rtp_to_nlp_delay{0};
    std::atomic<uint64_t> video_frame_delay{0};
    std::atomic<uint64_t> audio_frame_delay{0};
    
    // 性能统计
    std::atomic<uint64_t> decode_time{0};
    std::atomic<uint64_t> render_time{0};
    
    // 质量统计
    std::atomic<uint32_t> video_width{0};
    std::atomic<uint32_t> video_height{0};
    std::atomic<uint32_t> video_fps{0};
    std::atomic<uint32_t> video_bitrate{0};
    std::atomic<uint32_t> audio_bitrate{0};
};

#endif // WEBRTC_STATS_HPP 