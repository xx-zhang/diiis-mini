cmake_minimum_required(VERSION 3.12)
project(D3Server VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(BUILD_TESTS "Build the test suite" ON)
option(DEBUG_MODE "Enable debug mode" ON)

# Dependencies
find_package(Threads REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system) # Try finding only system first, Asio is often header-only with system linkage
# find_package(Boost REQUIRED COMPONENTS asio system) # Original line

# Define debug mode
if(DEBUG_MODE)
    add_definitions(-DDEBUG_MODE)
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    # ${PROJECT_SOURCE_DIR}/third_party
    ${SQLite3_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS} # Keep this one, as FindBoost traditionally sets it.
)

# Add subdirectories
add_subdirectory(src)

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif() 