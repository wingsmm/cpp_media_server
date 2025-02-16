# 设置协议转换器源文件
set(PROTOCOL_CONVERTER_SRCS
    protocol_manager.cpp
    rtmp2webrtc_converter.cpp
    webrtc2rtmp_converter.cpp
    protocol_converter.cpp
)

# 创建静态库
add_library(protocol_converter STATIC ${PROTOCOL_CONVERTER_SRCS})

# 添加依赖的源文件
target_sources(protocol_converter PRIVATE
    ${CMAKE_SOURCE_DIR}/src/utils/logger.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/data_buffer.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/byte_stream.cpp
    ${CMAKE_SOURCE_DIR}/src/format/flv/flv_demux.cpp
    ${CMAKE_SOURCE_DIR}/src/format/flv/flv_mux.cpp
)

# 添加测试目录
add_subdirectory(test) 