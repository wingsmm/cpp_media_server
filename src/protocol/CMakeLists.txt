cmake_minimum_required(VERSION 3.5)

# 添加子目录
add_subdirectory(converter)
add_subdirectory(server)
add_subdirectory(test)

# 主程序
add_executable(media_server main.cpp)

# 设置包含目录
target_include_directories(media_server PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# 链接依赖库
target_link_libraries(media_server
    protocol_converter
    protocol_server
    ${COMMON_LIBS}
)

# 设置输出路径
set_target_properties(media_server
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/objs"
) 