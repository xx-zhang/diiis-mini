add_library(game_server
    src/game_server.cpp
    src/world.cpp
    src/player.cpp
    src/monster.cpp
    src/item.cpp
    src/quest.cpp
    src/chat.cpp
    src/ai.cpp
    src/command_handler.cpp
)

target_include_directories(game_server
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(game_server
    PRIVATE
    core
    utils
    database
) 