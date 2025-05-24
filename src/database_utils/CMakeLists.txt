cmake_minimum_required(VERSION 3.10)

# Find SQLite package
find_package(SQLite3 REQUIRED)

# Add include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SQLite3_INCLUDE_DIRS}
)

# Define source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/database_utils/*.h")

# Create library
add_library(database_utils STATIC ${SOURCES} ${HEADERS})

# Link against SQLite
target_link_libraries(database_utils PRIVATE ${SQLite3_LIBRARIES})

# Add include directories
target_include_directories(database_utils PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SQLite3_INCLUDE_DIRS}
)

# Add dependencies
target_link_libraries(database_utils PUBLIC core)

# If using sqlite_orm, add it here (assuming it's in third_party)
# target_include_directories(database_utils PUBLIC ${CMAKE_SOURCE_DIR}/third_party/sqlite_orm/include) 
