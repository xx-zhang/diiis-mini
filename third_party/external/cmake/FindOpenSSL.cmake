# FindOpenSSL.cmake
# Find OpenSSL library

find_path(OPENSSL_INCLUDE_DIR openssl/ssl.h
    PATHS 
    /usr/include
    /usr/local/include
    ${PROJECT_SOURCE_DIR}/libs/openssl
    ${OPENSSL_ROOT_DIR}
    PATH_SUFFIXES include
)

find_library(OPENSSL_SSL_LIBRARY 
    NAMES ssl
    PATHS 
    /usr/lib
    /usr/local/lib
    ${PROJECT_SOURCE_DIR}/libs/openssl
    ${OPENSSL_ROOT_DIR}
    PATH_SUFFIXES lib
)

find_library(OPENSSL_CRYPTO_LIBRARY 
    NAMES crypto
    PATHS 
    /usr/lib
    /usr/local/lib
    ${PROJECT_SOURCE_DIR}/libs/openssl
    ${OPENSSL_ROOT_DIR}
    PATH_SUFFIXES lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OpenSSL DEFAULT_MSG 
    OPENSSL_SSL_LIBRARY 
    OPENSSL_CRYPTO_LIBRARY 
    OPENSSL_INCLUDE_DIR
)

mark_as_advanced(OPENSSL_INCLUDE_DIR OPENSSL_SSL_LIBRARY OPENSSL_CRYPTO_LIBRARY)

set(OPENSSL_LIBRARIES ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
set(OPENSSL_INCLUDE_DIRS ${OPENSSL_INCLUDE_DIR}) 