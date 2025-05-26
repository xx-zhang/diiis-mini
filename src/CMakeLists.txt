cmake_minimum_required(VERSION 3.16)

# 项目名称和语言
project(D3Server 
    VERSION 1.0.0 
    LANGUAGES CXX
)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 寻找必要的包
find_package(Boost REQUIRED COMPONENTS system)
find_package(Threads REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

# 添加全局包含目录 (相对于当前 CMakeLists.txt，即 /root/d3s/src)
include_directories(
    core/include
    database_utils/include
    battle_net/include
    game_server/include
    rest_api/include
    utils/include            # For utils headers like utils/debug.h
    ../third_party/include # spdlog 等第三方库
)

# 添加子目录
add_subdirectory(core)
add_subdirectory(database_utils)
add_subdirectory(battle_net)
add_subdirectory(game_server)
add_subdirectory(rest_api)

# 主可执行文件
add_executable(d3server main.cpp)

# d3server 目标需要能够找到各模块的公共头文件
# 这些路径应该与全局 include_directories 类似
target_include_directories(d3server
    PRIVATE
    core/include
    database_utils/include
    battle_net/include
    game_server/include
    rest_api/include
    utils/include            # For utils headers
    ../third_party/include # spdlog 等第三方库
)

# 链接库
target_link_libraries(d3server 
    PRIVATE
    core
    database_utils
    battle_net
    game_server
    rest_api
    Boost::system
    Threads::Threads
    SQLite::SQLite3
    OpenSSL::Crypto
    OpenSSL::SSL
)

# 编译器警告和优化
if(MSVC)
    target_compile_options(d3server PRIVATE /W4 /WX)
else()
    target_compile_options(d3server PRIVATE 
        -Wall 
        -Wextra 
        -Wpedantic
        -Werror
    )
endif()

# 安装
install(TARGETS d3server
    RUNTIME DESTINATION bin
)

# 启用测试
enable_testing() 
