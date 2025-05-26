cmake_minimum_required(VERSION 3.10)

# 定义库名
set(CORE_LIB_NAME core)

# spdlog 的包含目录 (相对于项目根目录 /root/d3s/src，所以是 ../third_party/include)
set(SPDLOG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/include)

# 定义源文件
file(GLOB_RECURSE CORE_SOURCES "src/*.cpp")
# 头文件通常不需要显式添加到 add_library，除非它们需要被安装或特殊处理
# file(GLOB_RECURSE CORE_HEADERS "${CMAKE_SOURCE_DIR}/include/core/*.h") 

# 创建库
add_library(${CORE_LIB_NAME} STATIC ${CORE_SOURCES})

# 设置 C++ 标准
target_compile_features(${CORE_LIB_NAME} PUBLIC cxx_std_17)

# 包含目录
target_include_directories(${CORE_LIB_NAME}
    PUBLIC
        # core 模块的公共头文件路径已经由顶层 CMakeLists.txt 的 include_directories() 提供
        # 这里主要暴露 core 模块依赖的第三方库的头文件路径
        ${SPDLOG_INCLUDE_DIR}
    PRIVATE
        # 如果 core 模块内部的 .cpp 文件需要一个特殊的相对路径来包含自己的头文件，
        # 例如，如果它们直接 #include "logger.h" 而不是 "core/logger.h"，
        # 则需要添加一个指向 src/include/core 的路径。
        # 但鉴于顶层已经添加了 include/core, 这通常不是必需的。
        # ${CMAKE_CURRENT_SOURCE_DIR}/../include/core # 示例，如果需要的话
)

# 编译选项
target_compile_options(${CORE_LIB_NAME} PRIVATE 
    -Wall 
    -Wextra 
    -Werror
)

# 如果 core 模块有其他依赖项，在这里链接
# target_link_libraries(${CORE_LIB_NAME} PRIVATE some_other_lib)

target_link_libraries(core
    PRIVATE
    utils
    database
    battle_net
    game_server
    rest_api
    ${OPENSSL_LIBRARIES}
) 
