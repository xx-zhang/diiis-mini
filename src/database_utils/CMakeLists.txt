cmake_minimum_required(VERSION 3.10)

# 设置 C++17 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(SQLite3 REQUIRED)

# Define source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)

file(GLOB_RECURSE HEADERS 
    "${CMAKE_SOURCE_DIR}/src/include/database_utils/*.h"
)

# Create library
add_library(database_utils STATIC ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(database_utils 
    PUBLIC 
    ${CMAKE_SOURCE_DIR}/src/include/database_utils
    ${CMAKE_SOURCE_DIR}/src/include/d3core
)

# Link against SQLite and core library
target_link_libraries(database_utils 
    PRIVATE 
    ${SQLite3_LIBRARIES}
    d3core
)

# Ensure C++17 standard
target_compile_features(database_utils PUBLIC cxx_std_17)

# Optional: Add compiler warnings
target_compile_options(database_utils PRIVATE 
    -Wall 
    -Wextra 
    -Werror
)

# If using sqlite_orm, add it here (assuming it's in third_party)
# target_include_directories(database_utils PUBLIC ${CMAKE_SOURCE_DIR}/third_party/sqlite_orm/include) 
