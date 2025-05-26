# FindSpdlog.cmake
# Find spdlog library

find_path(SPDLOG_INCLUDE_DIR spdlog/spdlog.h
    PATHS 
    /usr/include
    /usr/local/include
    ${PROJECT_SOURCE_DIR}/libs/spdlog
    ${SPDLOG_ROOT_DIR}
    PATH_SUFFIXES include
)

find_library(SPDLOG_LIBRARY 
    NAMES spdlog
    PATHS 
    /usr/lib
    /usr/local/lib
    ${PROJECT_SOURCE_DIR}/libs/spdlog
    ${SPDLOG_ROOT_DIR}
    PATH_SUFFIXES lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Spdlog DEFAULT_MSG SPDLOG_LIBRARY SPDLOG_INCLUDE_DIR)

mark_as_advanced(SPDLOG_INCLUDE_DIR SPDLOG_LIBRARY)

set(SPDLOG_LIBRARIES ${SPDLOG_LIBRARY})
set(SPDLOG_INCLUDE_DIRS ${SPDLOG_INCLUDE_DIR}) 