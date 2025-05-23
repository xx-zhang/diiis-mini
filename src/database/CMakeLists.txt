add_library(database
    src/database_manager.cpp
    src/account_dao.cpp
    src/character_dao.cpp
    src/world_dao.cpp
)

target_include_directories(database
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(database
    PRIVATE
    utils
    ${SQLite3_LIBRARIES}
) 