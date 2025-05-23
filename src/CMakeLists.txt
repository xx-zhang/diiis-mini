add_subdirectory(core)
add_subdirectory(utils)
add_subdirectory(database)
add_subdirectory(battle_net)
add_subdirectory(game_server)
add_subdirectory(rest_api)

# Main executable
add_executable(d3server main.cpp)

target_link_libraries(d3server 
    PRIVATE
    core
    utils
    database
    battle_net
    game_server
    rest_api
    ${SQLite3_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    Threads::Threads
)

# Installation
install(TARGETS d3server
    RUNTIME DESTINATION bin
) 