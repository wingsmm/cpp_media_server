#ifndef PROTOCOL_CONVERTER_HPP
#define PROTOCOL_CONVERTER_HPP

#include "utils/av/media_packet.hpp"
#include <memory>
#include <functional>
#include <chrono>

class ProtocolConverter {
public:
    // 错误码定义：定义所有可能的错误类型
    enum ErrorCode {
        ERROR_OK = 0,                    // 正常，无错误
        ERROR_INIT_FAILED = -1,          // 初始化失败
        ERROR_INVALID_PARAM = -2,        // 无效的参数
        ERROR_NOT_INITIALIZED = -3,       // 未初始化
        ERROR_CODEC_NOT_SUPPORTED = -4,   // 不支持的编码格式
        ERROR_PACKET_TOO_LARGE = -5,     // 数据包过大
        ERROR_STATE_INVALID = -6,        // 状态错误，非法的状态转换
        ERROR_BUFFER_EMPTY = -7,         // 缓冲区为空
        ERROR_CODEC_CHANGED = -8,        // 编码格式发生改变
        ERROR_CONFIG_INVALID = -9        // 配置参数无效
    };

    // 状态定义：定义转换器的工作状态
    enum State {
        STATE_IDLE = 0,            // 初始状态，等待初始化
        STATE_INITIALIZED = 1,      // 已初始化，等待数据输入
        STATE_RUNNING = 2,         // 正在运行，处理数据中
        STATE_ERROR = 3,           // 错误状态，需要恢复
        STATE_STOPPING = 4,        // 正在停止，清理资源中
        STATE_STOPPED = 5          // 已停止，可以销毁
    };

    // 状态转换定义：定义允许的状态转换路径
    enum StateTransition {
        TRANS_IDLE_TO_INIT,         // 从空闲到初始化
        TRANS_INIT_TO_RUNNING,      // 从初始化到运行
        TRANS_RUNNING_TO_ERROR,     // 从运行到错误
        TRANS_ERROR_TO_INIT,        // 从错误恢复到初始化
        TRANS_RUNNING_TO_STOPPING,  // 从运行到停止中
        TRANS_STOPPING_TO_STOPPED   // 从停止中到已停止
    };

    // 配置结构：定义转换器的配置参数
    struct Config {
        bool enable_video = true;                      // 是否启用视频
        bool enable_audio = true;                      // 是否启用音频
        size_t max_video_buffer_size = 1024*1024;     // 视频缓冲区最大大小（1MB）
        size_t max_audio_buffer_size = 128*1024;      // 音频缓冲区最大大小（128KB）
    };

    // 统计信息结构：记录运行时的各项统计数据
    struct Statistics {
        // 基本计数：记录处理的数据包数量
        uint64_t total_packets = 0;      // 总处理包数
        uint64_t video_packets = 0;      // 视频包数
        uint64_t audio_packets = 0;      // 音频包数
        uint64_t error_packets = 0;      // 错误包数

        // 带宽统计：记录数据传输量
        uint64_t total_bytes = 0;        // 总传输字节数
        uint64_t video_bytes = 0;        // 视频数据字节数
        uint64_t audio_bytes = 0;        // 音频数据字节数
        float current_bitrate = 0.0f;    // 当前比特率 (bps)
        float average_bitrate = 0.0f;    // 平均比特率 (bps)

        // 时间统计
        uint64_t first_packet_time = 0;  // 第一个包的时间戳
        uint64_t last_packet_time = 0;   // 最后一个包的时间戳
        uint64_t total_process_time = 0; // 总处理时间(微秒)
        float average_process_time = 0.0f;// 平均处理时间(微秒)

        // 错误统计
        uint32_t init_failures = 0;      // 初始化失败次数
        uint32_t state_errors = 0;       // 状态错误次数
        uint32_t codec_errors = 0;       // 编码错误次数
        uint32_t buffer_errors = 0;      // 缓冲区错误次数
    };

    // 资源使用统计结构
    struct ResourceUsage {
        size_t video_buffer_used = 0;    // 视频缓冲区使用量
        size_t audio_buffer_used = 0;    // 音频缓冲区使用量
        size_t peak_video_buffer = 0;    // 视频缓冲区峰值
        size_t peak_audio_buffer = 0;    // 音频缓冲区峰值
        uint64_t total_packets_processed = 0; // 已处理包总数
    };

    virtual ~ProtocolConverter() = default;

    // 初始化转换器
    virtual bool init(const Config& config) = 0;

    // 输入数据包
    virtual int input_packet(MEDIA_PACKET_PTR pkt_ptr) = 0;

    // 设置输出回调
    virtual void set_output_callback(std::function<void(MEDIA_PACKET_PTR)> cb) {
        output_callback_ = cb;
    }

    // 状态查询接口
    State get_state() const { return state_; }
    ErrorCode get_last_error() const { return last_error_; }

    // 重置接口
    virtual void reset() {
        state_ = STATE_IDLE;
        last_error_ = ERROR_OK;
        config_ = Config();
    }

    // 清理接口
    virtual void cleanup() {
        output_callback_ = nullptr;
        reset();
    }

    // 获取资源使用情况
    virtual ResourceUsage get_resource_usage() const {
        return resource_usage_;
    }

    // 获取统计信息
    virtual const Statistics& get_statistics() const {
        return stats_;
    }

    // 重置统计信息
    virtual void reset_statistics() {
        stats_ = Statistics();
    }

protected:
    std::function<void(MEDIA_PACKET_PTR)> output_callback_;
    State state_ = STATE_IDLE;
    ErrorCode last_error_ = ERROR_OK;
    Config config_;
    ResourceUsage resource_usage_;
    Statistics stats_;

    // 状态转换检查函数
    bool check_state_transition(StateTransition trans);

    // 状态转换函数
    bool transition_state(StateTransition trans);

    // 错误处理辅助函数
    void set_error(ErrorCode error);

    // 更新资源使用统计
    void update_resource_usage(const MEDIA_PACKET_PTR& pkt);

    // 更新统计信息
    void update_statistics(const MEDIA_PACKET_PTR& pkt, uint64_t process_time);

    // 更新错误统计
    void update_error_statistics(ErrorCode error);

private:
    // 获取当前时间(微秒)
    uint64_t get_current_time();
};

#endif 