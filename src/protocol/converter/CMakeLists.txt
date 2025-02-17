# 设置协议转换器源文件
set(CONVERTER_SRCS
    protocol_converter.cpp
    protocol_manager.cpp
    rtmp2webrtc_converter.cpp
    webrtc2rtmp_converter.cpp
)

# 创建静态库
add_library(protocol_converter STATIC ${CONVERTER_SRCS})

# 添加头文件搜索路径
target_include_directories(protocol_converter
    PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

# 链接依赖库
target_link_libraries(protocol_converter
    utils_common
    ${COMMON_LIBS}
) 