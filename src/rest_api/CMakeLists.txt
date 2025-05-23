add_library(rest_api
    src/rest_server.cpp
    src/account_controller.cpp
    src/admin_controller.cpp
)

target_include_directories(rest_api
    PUBLIC
    include
)

target_link_libraries(rest_api
    core
    utils
    database
    Boost::boost
    Boost::system
) 