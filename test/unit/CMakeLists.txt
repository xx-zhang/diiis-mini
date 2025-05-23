include(GoogleTest)

# Core module tests
add_executable(core_tests
    core_tests.cpp
    config_tests.cpp
    logger_tests.cpp
)

target_link_libraries(core_tests
    PRIVATE
    core
    utils
    GTest::GTest
    GTest::Main
)

gtest_discover_tests(core_tests)

# Database module tests
add_executable(database_tests
    database_tests.cpp
    account_dao_tests.cpp
    character_dao_tests.cpp
)

target_link_libraries(database_tests
    PRIVATE
    database
    utils
    GTest::GTest
    GTest::Main
)

gtest_discover_tests(database_tests)

# Battle.net module tests
add_executable(battle_net_tests
    battle_net_tests.cpp
    auth_tests.cpp
)

target_link_libraries(battle_net_tests
    PRIVATE
    battle_net
    utils
    GTest::GTest
    GTest::Main
)

gtest_discover_tests(battle_net_tests)

# Game server module tests
add_executable(game_server_tests
    game_server_tests.cpp
    world_tests.cpp
    player_tests.cpp
)

target_link_libraries(game_server_tests
    PRIVATE
    game_server
    utils
    GTest::GTest
    GTest::Main
)

gtest_discover_tests(game_server_tests)

# REST API module tests
add_executable(rest_api_tests
    rest_api_tests.cpp
    account_controller_tests.cpp
)

target_link_libraries(rest_api_tests
    PRIVATE
    rest_api
    utils
    GTest::GTest
    GTest::Main
)

gtest_discover_tests(rest_api_tests) 