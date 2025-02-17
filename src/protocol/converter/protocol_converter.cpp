#include "protocol_converter.hpp"
#include "utils/logger.hpp"

bool ProtocolConverter::check_state_transition(StateTransition trans) {
    switch (trans) {
        case TRANS_IDLE_TO_INIT:
            return state_ == STATE_IDLE;
        case TRANS_INIT_TO_RUNNING:
            return state_ == STATE_INITIALIZED;
        case TRANS_RUNNING_TO_ERROR:
            return state_ == STATE_RUNNING;
        case TRANS_ERROR_TO_INIT:
            return state_ == STATE_ERROR;
        case TRANS_RUNNING_TO_STOPPING:
            return state_ == STATE_RUNNING;
        case TRANS_STOPPING_TO_STOPPED:
            return state_ == STATE_STOPPING;
        default:
            return false;
    }
}

bool ProtocolConverter::transition_state(StateTransition trans) {
    if (!check_state_transition(trans)) {
        log_errorf("Invalid state transition: %d -> %d", state_, trans);
        set_error(ERROR_STATE_INVALID);
        return false;
    }

    switch (trans) {
        case TRANS_IDLE_TO_INIT:
            state_ = STATE_INITIALIZED;
            break;
        case TRANS_INIT_TO_RUNNING:
            state_ = STATE_RUNNING;
            break;
        case TRANS_RUNNING_TO_ERROR:
            state_ = STATE_ERROR;
            break;
        case TRANS_ERROR_TO_INIT:
            state_ = STATE_INITIALIZED;
            break;
        case TRANS_RUNNING_TO_STOPPING:
            state_ = STATE_STOPPING;
            break;
        case TRANS_STOPPING_TO_STOPPED:
            state_ = STATE_STOPPED;
            break;
    }
    return true;
}

void ProtocolConverter::set_error(ErrorCode error) {
    last_error_ = error;
    if (error != ERROR_OK) {
        state_ = STATE_ERROR;
        log_errorf("Protocol converter error: %d", error);
    }
}

void ProtocolConverter::update_resource_usage(const MEDIA_PACKET_PTR& pkt) {
    if (!pkt || !pkt->buffer_ptr_) return;

    resource_usage_.total_packets_processed++;
    
    size_t buffer_size = pkt->buffer_ptr_->data_len();
    if (pkt->av_type_ == MEDIA_VIDEO_TYPE) {
        resource_usage_.video_buffer_used = buffer_size;
        if (buffer_size > resource_usage_.peak_video_buffer) {
            resource_usage_.peak_video_buffer = buffer_size;
        }
    } else if (pkt->av_type_ == MEDIA_AUDIO_TYPE) {
        resource_usage_.audio_buffer_used = buffer_size;
        if (buffer_size > resource_usage_.peak_audio_buffer) {
            resource_usage_.peak_audio_buffer = buffer_size;
        }
    }
}

void ProtocolConverter::update_statistics(const MEDIA_PACKET_PTR& pkt, uint64_t process_time) {
    if (!pkt || !pkt->buffer_ptr_) return;

    // 更新基本计数
    stats_.total_packets++;
    if (pkt->av_type_ == MEDIA_VIDEO_TYPE) {
        stats_.video_packets++;
        stats_.video_bytes += pkt->buffer_ptr_->data_len();
    } else if (pkt->av_type_ == MEDIA_AUDIO_TYPE) {
        stats_.audio_packets++;
        stats_.audio_bytes += pkt->buffer_ptr_->data_len();
    }
    stats_.total_bytes += pkt->buffer_ptr_->data_len();

    // 更新时间统计
    uint64_t current_time = get_current_time();
    if (stats_.first_packet_time == 0) {
        stats_.first_packet_time = current_time;
    }
    stats_.last_packet_time = current_time;
    stats_.total_process_time += process_time;
    stats_.average_process_time = static_cast<float>(stats_.total_process_time) / stats_.total_packets;

    // 更新带宽统计
    if (stats_.last_packet_time > stats_.first_packet_time) {
        float duration = (stats_.last_packet_time - stats_.first_packet_time) / 1000000.0f;
        stats_.average_bitrate = (stats_.total_bytes * 8) / duration;
    }
}

void ProtocolConverter::update_error_statistics(ErrorCode error) {
    stats_.error_packets++;
    switch (error) {
        case ERROR_INIT_FAILED:
            stats_.init_failures++;
            break;
        case ERROR_STATE_INVALID:
            stats_.state_errors++;
            break;
        case ERROR_CODEC_NOT_SUPPORTED:
        case ERROR_CODEC_CHANGED:
            stats_.codec_errors++;
            break;
        case ERROR_BUFFER_EMPTY:
        case ERROR_PACKET_TOO_LARGE:
            stats_.buffer_errors++;
            break;
        default:
            break;
    }
}

uint64_t ProtocolConverter::get_current_time() {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(
        now.time_since_epoch()).count();
} 