add_library(utils
    src/string_utils.cpp
    src/file_utils.cpp
    src/crypto_utils.cpp
    src/debug.cpp
)

target_include_directories(utils
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
))

target_link_libraries(utils
    PRIVATE
    ${OPENSSL_LIBRARIES}
) 
