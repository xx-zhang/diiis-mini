add_library(database
    src/database_manager.cpp
)

target_include_directories(database
    PUBLIC # For other modules to find database_manager.h
    include # This should allow #include "database_manager.h"
)

target_link_libraries(database
    core
    utils
    SQLite::SQLite3
) 