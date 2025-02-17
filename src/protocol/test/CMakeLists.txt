# 查找 pthread 库
find_package(Threads REQUIRED)

# 测试源文件
set(TEST_SRCS
    boundary_test.cpp
    concurrency_test.cpp
    error_handling_test.cpp
    integration_test.cpp
    logger_test.cpp
    performance_test.cpp
    rtmp_to_webrtc_test.cpp
)

# 为每个测试文件创建可执行文件
foreach(test_src ${TEST_SRCS})
    get_filename_component(test_name ${test_src} NAME_WE)
    add_executable(${test_name} ${test_src})
    
    # 添加头文件搜索路径
    target_include_directories(${test_name}
        PRIVATE
        ${CMAKE_SOURCE_DIR}/src
    )
    
    # 链接依赖库
    target_link_libraries(${test_name}
        protocol_converter
        utils_common
        ${COMMON_LIBS}
        pthread
    )
endforeach()

# 设置输出路径
set_target_properties(
    logger_test 
    rtmp_to_webrtc_test 
    error_handling_test 
    boundary_test 
    concurrency_test
    performance_test
    integration_test
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/objs"
)