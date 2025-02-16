#include "../protocol_manager.hpp"
#include "utils/logger.hpp"
#include "utils/data_buffer.hpp"
#include <iostream>
#include <thread>

void test_rtmp_to_webrtc() {
    // 创建 RTMP 到 WebRTC 的转换器
    auto converter = ProtocolManager::get_instance().create_converter("rtmp", "webrtc");
    if (!converter) {
        log_errorf("Failed to create protocol converter");
        return;
    }

    // 设置输出回调
    converter->set_output_callback([](MEDIA_PACKET_PTR pkt) {
        log_infof("Converted packet - av type:%s, codec type:%s, dts:%ld, pts:%ld, len:%lu",
            avtype_tostring(pkt->av_type_).c_str(),
            codectype_tostring(pkt->codec_type_).c_str(),
            pkt->dts_,
            pkt->pts_,
            pkt->buffer_ptr_->data_len());
    });

    // 创建测试视频包
    MEDIA_PACKET_PTR video_pkt = std::make_shared<MEDIA_PACKET>();
    video_pkt->av_type_ = MEDIA_VIDEO_TYPE;
    video_pkt->codec_type_ = MEDIA_CODEC_H264;
    video_pkt->fmt_type_ = MEDIA_FORMAT_RAW;
    video_pkt->dts_ = 0;
    video_pkt->pts_ = 0;
    video_pkt->is_key_frame_ = true;
    video_pkt->is_seq_hdr_ = false;

    // 添加测试视频数据
    uint8_t test_video_data[] = {0x00, 0x00, 0x00, 0x01, 0x67, 0x42, 0x00, 0x1E}; // 模拟 H.264 SPS
    video_pkt->buffer_ptr_->append_data((char*)test_video_data, sizeof(test_video_data));

    // 测试视频转换
    log_infof("Testing video packet conversion...");
    int ret = converter->input_packet(video_pkt);
    log_infof("Video packet conversion result: %d", ret);
}

void test_webrtc_to_rtmp() {
    // 创建 WebRTC 到 RTMP 的转换器
    auto converter = ProtocolManager::get_instance().create_converter("webrtc", "rtmp");
    if (!converter) {
        log_errorf("Failed to create protocol converter");
        return;
    }

    // 设置输出回调
    converter->set_output_callback([](MEDIA_PACKET_PTR pkt) {
        log_infof("Converted packet - av type:%s, codec type:%s, dts:%ld, pts:%ld, len:%lu",
            avtype_tostring(pkt->av_type_).c_str(),
            codectype_tostring(pkt->codec_type_).c_str(),
            pkt->dts_,
            pkt->pts_,
            pkt->buffer_ptr_->data_len());
    });

    // TODO: 添加 WebRTC 测试包
    // 这里需要根据实际 WebRTC 包格式添加测试数据
}

void test_error_handling() {
    log_infof("Testing error handling...");
    
    auto converter = ProtocolManager::get_instance().create_converter("rtmp", "webrtc");
    if (!converter) {
        log_errorf("Failed to create converter");
        return;
    }

    // 测试未初始化状态
    MEDIA_PACKET_PTR pkt = std::make_shared<MEDIA_PACKET>();
    int ret = converter->input_packet(pkt);
    log_infof("Uninitialized test result: %d (expected: %d)",
              ret, ProtocolConverter::ERROR_NOT_INITIALIZED);

    // 测试无效参数
    ret = converter->input_packet(nullptr);
    log_infof("Invalid param test result: %d (expected: %d)",
              ret, ProtocolConverter::ERROR_INVALID_PARAM);

    // 测试配置参数
    ProtocolConverter::Config config;
    config.enable_video = false;
    config.max_video_buffer_size = 1024;
    converter->init(config);

    // 测试缓冲区溢出
    pkt->av_type_ = MEDIA_VIDEO_TYPE;
    pkt->buffer_ptr_ = std::make_shared<data_buffer>();
    uint8_t test_data[2048] = {0};
    pkt->buffer_ptr_->append_data((char*)test_data, 2048);
    ret = converter->input_packet(pkt);
    log_infof("Buffer overflow test result: %d (expected: %d)",
              ret, ProtocolConverter::ERROR_PACKET_TOO_LARGE);
}

void test_state_transitions() {
    log_infof("Testing state transitions...");
    
    auto converter = ProtocolManager::get_instance().create_converter("rtmp", "webrtc");
    if (!converter) {
        log_errorf("Failed to create converter");
        return;
    }

    // 测试初始状态
    log_infof("Initial state: %d", converter->get_state());

    // 测试无效配置
    ProtocolConverter::Config invalid_config;
    invalid_config.enable_video = false;
    invalid_config.enable_audio = false;
    bool ret = converter->init(invalid_config);
    log_infof("Invalid config test result: %d, state: %d, error: %d",
              ret, converter->get_state(), converter->get_last_error());

    // 测试有效配置
    ProtocolConverter::Config valid_config;
    ret = converter->init(valid_config);
    log_infof("Valid config test result: %d, state: %d",
              ret, converter->get_state());

    // 测试重复初始化
    ret = converter->init(valid_config);
    log_infof("Duplicate init test result: %d, state: %d, error: %d",
              ret, converter->get_state(), converter->get_last_error());

    // 测试空包
    MEDIA_PACKET_PTR empty_pkt = std::make_shared<MEDIA_PACKET>();
    empty_pkt->buffer_ptr_ = std::make_shared<data_buffer>();
    int pkt_ret = converter->input_packet(empty_pkt);
    log_infof("Empty packet test result: %d, state: %d, error: %d",
              pkt_ret, converter->get_state(), converter->get_last_error());

    // 测试不支持的编码格式
    MEDIA_PACKET_PTR invalid_codec_pkt = std::make_shared<MEDIA_PACKET>();
    invalid_codec_pkt->av_type_ = MEDIA_VIDEO_TYPE;
    invalid_codec_pkt->codec_type_ = MEDIA_CODEC_VP8;  // 不支持的编码格式
    invalid_codec_pkt->buffer_ptr_ = std::make_shared<data_buffer>();
    uint8_t test_data[] = {0x00};
    invalid_codec_pkt->buffer_ptr_->append_data((char*)test_data, sizeof(test_data));
    pkt_ret = converter->input_packet(invalid_codec_pkt);
    log_infof("Invalid codec test result: %d, state: %d, error: %d",
              pkt_ret, converter->get_state(), converter->get_last_error());
}

void test_resource_management() {
    log_infof("Testing resource management...");
    
    auto converter = ProtocolManager::get_instance().create_converter("rtmp", "webrtc");
    if (!converter) {
        log_errorf("Failed to create converter");
        return;
    }

    // 初始化转换器
    ProtocolConverter::Config config;
    bool ret = converter->init(config);
    log_infof("Init result: %d", ret);

    // 测试视频资源使用
    MEDIA_PACKET_PTR video_pkt = std::make_shared<MEDIA_PACKET>();
    video_pkt->av_type_ = MEDIA_VIDEO_TYPE;
    video_pkt->buffer_ptr_ = std::make_shared<data_buffer>();
    uint8_t video_data[1024] = {0};
    video_pkt->buffer_ptr_->append_data((char*)video_data, sizeof(video_data));
    converter->input_packet(video_pkt);

    // 测试音频资源使用
    MEDIA_PACKET_PTR audio_pkt = std::make_shared<MEDIA_PACKET>();
    audio_pkt->av_type_ = MEDIA_AUDIO_TYPE;
    audio_pkt->buffer_ptr_ = std::make_shared<data_buffer>();
    uint8_t audio_data[256] = {0};
    audio_pkt->buffer_ptr_->append_data((char*)audio_data, sizeof(audio_data));
    converter->input_packet(audio_pkt);

    // 检查资源使用情况
    auto usage = converter->get_resource_usage();
    log_infof("Resource usage - video: %lu, audio: %lu, total packets: %lu",
              usage.video_buffer_used,
              usage.audio_buffer_used,
              usage.total_packets_processed);

    // 测试重置
    converter->reset();
    log_infof("After reset - state: %d, error: %d",
              converter->get_state(),
              converter->get_last_error());

    // 测试清理
    converter->cleanup();
    usage = converter->get_resource_usage();
    log_infof("After cleanup - video: %lu, audio: %lu, total packets: %lu",
              usage.video_buffer_used,
              usage.audio_buffer_used,
              usage.total_packets_processed);
}

void test_statistics() {
    log_infof("Testing statistics...");
    
    auto converter = ProtocolManager::get_instance().create_converter("rtmp", "webrtc");
    if (!converter) {
        log_errorf("Failed to create converter");
        return;
    }

    // 初始化
    ProtocolConverter::Config config;
    converter->init(config);

    // 发送一些测试包
    for (int i = 0; i < 10; i++) {
        // 视频包
        MEDIA_PACKET_PTR video_pkt = std::make_shared<MEDIA_PACKET>();
        video_pkt->av_type_ = MEDIA_VIDEO_TYPE;
        video_pkt->buffer_ptr_ = std::make_shared<data_buffer>();
        uint8_t video_data[1024] = {0};
        video_pkt->buffer_ptr_->append_data((char*)video_data, sizeof(video_data));
        converter->input_packet(video_pkt);

        // 音频包
        MEDIA_PACKET_PTR audio_pkt = std::make_shared<MEDIA_PACKET>();
        audio_pkt->av_type_ = MEDIA_AUDIO_TYPE;
        audio_pkt->buffer_ptr_ = std::make_shared<data_buffer>();
        uint8_t audio_data[256] = {0};
        audio_pkt->buffer_ptr_->append_data((char*)audio_data, sizeof(audio_data));
        converter->input_packet(audio_pkt);

        // 模拟一些处理时间
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // 检查统计信息
    auto stats = converter->get_statistics();
    log_infof("Statistics:\n"
              "Total packets: %lu (video: %lu, audio: %lu, errors: %lu)\n"
              "Total bytes: %lu (video: %lu, audio: %lu)\n"
              "Average bitrate: %.2f bps\n"
              "Average process time: %.2f us\n"
              "Error counts - init: %u, state: %u, codec: %u, buffer: %u",
              stats.total_packets, stats.video_packets, stats.audio_packets, stats.error_packets,
              stats.total_bytes, stats.video_bytes, stats.audio_bytes,
              stats.average_bitrate,
              stats.average_process_time,
              stats.init_failures, stats.state_errors, stats.codec_errors, stats.buffer_errors);

    // 测试统计重置
    converter->reset_statistics();
    stats = converter->get_statistics();
    log_infof("After reset - total packets: %lu, total bytes: %lu",
              stats.total_packets, stats.total_bytes);
}

// 测试边界条件
void test_edge_cases() {
    log_infof("Testing edge cases...");
    
    auto converter = ProtocolManager::get_instance().create_converter("rtmp", "webrtc");
    if (!converter) {
        log_errorf("Failed to create converter");
        return;
    }

    // 测试极限配置
    ProtocolConverter::Config extreme_config;
    extreme_config.max_video_buffer_size = SIZE_MAX;  // 最大缓冲区
    extreme_config.max_audio_buffer_size = 0;         // 最小缓冲区
    bool ret = converter->init(extreme_config);
    log_infof("Extreme config test result: %d", ret);

    // 测试零大小数据包
    MEDIA_PACKET_PTR empty_pkt = std::make_shared<MEDIA_PACKET>();
    empty_pkt->av_type_ = MEDIA_VIDEO_TYPE;
    empty_pkt->buffer_ptr_ = std::make_shared<data_buffer>();
    int pkt_ret = converter->input_packet(empty_pkt);
    log_infof("Empty packet test result: %d", pkt_ret);

    // 测试快速切换状态
    for (int i = 0; i < 10; i++) {
        converter->cleanup();
        converter->init(ProtocolConverter::Config());
    }
    log_infof("After rapid state changes - state: %d, error: %d",
              converter->get_state(), converter->get_last_error());
}

// 测试异常情况
void test_error_scenarios() {
    log_infof("Testing error scenarios...");
    
    auto converter = ProtocolManager::get_instance().create_converter("rtmp", "webrtc");
    if (!converter) {
        log_errorf("Failed to create converter");
        return;
    }

    // 测试未初始化就设置回调
    converter->set_output_callback([](MEDIA_PACKET_PTR pkt) {
        log_infof("Unexpected callback");
    });

    // 测试无效的编码类型
    MEDIA_PACKET_PTR invalid_codec_pkt = std::make_shared<MEDIA_PACKET>();
    invalid_codec_pkt->av_type_ = MEDIA_VIDEO_TYPE;
    invalid_codec_pkt->codec_type_ = static_cast<MEDIA_CODEC_TYPE>(999); // 无效的编码类型
    invalid_codec_pkt->buffer_ptr_ = std::make_shared<data_buffer>();
    uint8_t test_data[] = {0x00};
    invalid_codec_pkt->buffer_ptr_->append_data((char*)test_data, sizeof(test_data));
    int ret = converter->input_packet(invalid_codec_pkt);
    log_infof("Invalid codec type test result: %d", ret);

    // 测试错误的数据包顺序
    ProtocolConverter::Config config;
    converter->init(config);

    // 发送音频包但没有之前的配置包
    MEDIA_PACKET_PTR audio_pkt = std::make_shared<MEDIA_PACKET>();
    audio_pkt->av_type_ = MEDIA_AUDIO_TYPE;
    audio_pkt->codec_type_ = MEDIA_CODEC_AAC;
    audio_pkt->buffer_ptr_ = std::make_shared<data_buffer>();
    audio_pkt->buffer_ptr_->append_data((char*)test_data, sizeof(test_data));
    ret = converter->input_packet(audio_pkt);
    log_infof("Wrong packet order test result: %d", ret);
}

// 测试并发情况
void test_concurrent_access() {
    log_infof("Testing concurrent access...");
    
    auto converter = ProtocolManager::get_instance().create_converter("rtmp", "webrtc");
    if (!converter) {
        log_errorf("Failed to create converter");
        return;
    }

    ProtocolConverter::Config config;
    converter->init(config);

    // 创建多个线程同时发送数据包
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; i++) {
        threads.emplace_back([converter, i]() {
            for (int j = 0; j < 100; j++) {
                MEDIA_PACKET_PTR pkt = std::make_shared<MEDIA_PACKET>();
                pkt->av_type_ = (j % 2) ? MEDIA_VIDEO_TYPE : MEDIA_AUDIO_TYPE;
                pkt->buffer_ptr_ = std::make_shared<data_buffer>();
                uint8_t test_data[128] = {0};
                pkt->buffer_ptr_->append_data((char*)test_data, sizeof(test_data));
                converter->input_packet(pkt);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    // 检查最终状态
    auto stats = converter->get_statistics();
    log_infof("After concurrent test - packets: %lu, errors: %lu",
              stats.total_packets, stats.error_packets);
}

int main(int argc, char* argv[]) {
    // 设置日志
    Logger::get_instance()->set_filename("protocol_converter_test.log");
    Logger::get_instance()->set_level(LOGGER_DEBUG_LEVEL);

    log_infof("Starting protocol converter tests...");

    // 测试 RTMP 到 WebRTC 的转换
    log_infof("Testing RTMP to WebRTC conversion:");
    test_rtmp_to_webrtc();

    // 测试 WebRTC 到 RTMP 的转换
    log_infof("Testing WebRTC to RTMP conversion:");
    test_webrtc_to_rtmp();

    // 测试错误处理
    test_error_handling();

    // 测试状态转换
    test_state_transitions();

    // 添加资源管理测试
    test_resource_management();

    // 添加统计信息测试
    test_statistics();

    // 添加新的测试用例
    test_edge_cases();
    test_error_scenarios();
    test_concurrent_access();

    log_infof("Protocol converter tests completed.");
    return 0;
} 