add_library(game_server
    src/game_server.cpp
    src/player.cpp       # Added
    src/world.cpp        # Added
    src/item.cpp         # Added
    # src/monster.cpp      # Placeholder, if you create it
    # src/quest.cpp        # Placeholder, if you create it
    # src/chat.cpp         # Placeholder, if you create it
    # src/ai.cpp           # Placeholder, if you create it
    # src/command_handler.cpp # Placeholder, if you create it
)

target_include_directories(game_server
    PUBLIC
    include # Simplified path
)

target_link_libraries(game_server
    # PRIVATE # Not necessary for public dependencies
    core
    utils
    database
) 