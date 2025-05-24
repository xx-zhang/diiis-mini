cmake_minimum_required(VERSION 3.10)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add subdirectories for each component
add_subdirectory(core)
add_subdirectory(database_utils) # Merged database and utils
add_subdirectory(battle_net)
add_subdirectory(game_server)
add_subdirectory(rest_api)

# Main executable
add_executable(d3server main.cpp)

target_include_directories(d3server
    PRIVATE
    # Explicitly add include directories from our modules
    core/include
    database_utils/include
    battle_net/include
    game_server/include
    rest_api/include
)

target_link_libraries(d3server 
    PRIVATE
    core
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
