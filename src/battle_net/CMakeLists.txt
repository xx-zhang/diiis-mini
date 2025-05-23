add_library(battle_net
    src/battle_net_server.cpp
    src/auth_server.cpp
    src/protocol.cpp
    src/session.cpp
)

target_include_directories(battle_net
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(battle_net
    PRIVATE
    core
    utils
    database
) 