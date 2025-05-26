cmake_minimum_required(VERSION 3.10)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directories globally
include_directories(
    ${CMAKE_SOURCE_DIR}/src/include/d3core
    ${CMAKE_SOURCE_DIR}/src/include/database_utils
    ${CMAKE_SOURCE_DIR}/src/include/battle_net
    ${CMAKE_SOURCE_DIR}/src/include/game_server
    ${CMAKE_SOURCE_DIR}/src/include/rest_api
)

# Add subdirectories for each component
add_subdirectory(d3core)
add_subdirectory(database_utils)
add_subdirectory(battle_net)
add_subdirectory(game_server)
add_subdirectory(rest_api)

# Main executable
add_executable(d3server main.cpp)

# Explicitly set include directories for the main executable
target_include_directories(d3server
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src/include/d3core
    ${CMAKE_SOURCE_DIR}/src/include/database_utils
    ${CMAKE_SOURCE_DIR}/src/include/battle_net
    ${CMAKE_SOURCE_DIR}/src/include/game_server
    ${CMAKE_SOURCE_DIR}/src/include/rest_api
)

# Link libraries
target_link_libraries(d3server 
    PRIVATE
    d3core
    database_utils
    battle_net
    game_server
    rest_api
    Boost::system
    Boost::boost
    Threads::Threads
    SQLite::SQLite3
    OpenSSL::Crypto
    OpenSSL::SSL
)

# Installation
install(TARGETS d3server
    RUNTIME DESTINATION bin
) 
