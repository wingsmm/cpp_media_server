# 创建测试可执行文件
add_executable(protocol_converter_test
    ${CMAKE_CURRENT_SOURCE_DIR}/protocol_converter_test.cpp
)

# 查找 pthread 库
find_package(Threads REQUIRED)

# 添加依赖的源文件
target_sources(protocol_converter_test PRIVATE
    ${CMAKE_SOURCE_DIR}/src/utils/logger.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/data_buffer.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/byte_stream.cpp
    ${CMAKE_SOURCE_DIR}/src/format/flv/flv_demux.cpp
    ${CMAKE_SOURCE_DIR}/src/format/flv/flv_mux.cpp
)

# 链接依赖库
target_link_libraries(protocol_converter_test
    protocol_converter
    ${CMAKE_THREAD_LIBS_INIT}
)

# 设置输出路径
set_target_properties(protocol_converter_test
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/objs"
) 