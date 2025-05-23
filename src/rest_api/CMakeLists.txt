add_library(rest_api
    src/rest_server.cpp
    src/account_controller.cpp
    src/admin_controller.cpp
    src/http_utils.cpp
)

target_include_directories(rest_api
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(rest_api
    PRIVATE
    core
    utils
    database
) 