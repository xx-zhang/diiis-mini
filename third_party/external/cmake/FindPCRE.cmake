# FindPCRE.cmake
# Find PCRE library

find_path(PCRE_INCLUDE_DIR pcre.h
    PATHS 
    /usr/include
    /usr/local/include
    ${PROJECT_SOURCE_DIR}/libs/pcre
    ${PCRE_ROOT_DIR}
    PATH_SUFFIXES include
)

find_library(PCRE_LIBRARY 
    NAMES pcre
    PATHS 
    /usr/lib
    /usr/local/lib
    ${PROJECT_SOURCE_DIR}/libs/pcre
    ${PCRE_ROOT_DIR}
    PATH_SUFFIXES lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PCRE DEFAULT_MSG PCRE_LIBRARY PCRE_INCLUDE_DIR)

mark_as_advanced(PCRE_INCLUDE_DIR PCRE_LIBRARY)

set(PCRE_LIBRARIES ${PCRE_LIBRARY})
set(PCRE_INCLUDE_DIRS ${PCRE_INCLUDE_DIR}) 