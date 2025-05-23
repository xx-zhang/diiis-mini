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
    Threads::Threads
)

gtest_discover_tests(core_tests)

# Database module tests
add_executable(database_tests
    database/test_database_manager.cpp
)

target_link_libraries(database_tests
    PRIVATE
    database
    utils
    core
    GTest::GTest
    GTest::Main
    Threads::Threads
    stdc++fs
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
    core
    GTest::GTest
    GTest::Main
    Threads::Threads
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
    core
    GTest::GTest
    GTest::Main
    Threads::Threads
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
    core
    Boost::boost
    Boost::system
    GTest::GTest
    GTest::Main
    Threads::Threads
)

gtest_discover_tests(rest_api_tests)

# Utils module tests (if you have them)
# add_executable(utils_tests
#     utils_tests.cpp
#     string_utils_tests.cpp
#     crypto_utils_tests.cpp
# )
# target_link_libraries(utils_tests PRIVATE utils core Boost::boost Boost::system GTest::GTest GTest::Main Threads::Threads)
# gtest_discover_tests(utils_tests) 