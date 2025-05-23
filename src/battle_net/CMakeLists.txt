add_library(battle_net
    src/battle_net_server.cpp
    src/battle_net_client.cpp
)

target_include_directories(battle_net
    PUBLIC
    include
)

target_link_libraries(battle_net
    core
    utils
    database
) 