# 服务器源文件
set(SERVER_SRCS
    media_server.cpp
    rtmp_server.cpp
    webrtc_server.cpp
    webrtc_peer.cpp
    server_config.cpp
    ${CMAKE_SOURCE_DIR}/src/net/http/http_server.cpp
    ${CMAKE_SOURCE_DIR}/src/net/http/http_session.cpp
)

# 创建静态库
add_library(protocol_server STATIC ${SERVER_SRCS})

# 添加头文件搜索路径
target_include_directories(protocol_server
    PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)

# 链接依赖库
target_link_libraries(protocol_server
    protocol_converter
    utils_common
    uv
    ssl
    crypto
    dl
    pthread
    ${COMMON_LIBS}
) 