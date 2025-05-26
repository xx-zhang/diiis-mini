# FindSQLiteCpp.cmake
# Find SQLiteCpp library

find_path(SQLITECPP_INCLUDE_DIR SQLiteCpp/SQLiteCpp.h
    PATHS 
    /usr/include
    /usr/local/include
    ${PROJECT_SOURCE_DIR}/libs/SQLiteCpp
    ${SQLITECPP_ROOT_DIR}
    PATH_SUFFIXES include
)

find_library(SQLITECPP_LIBRARY 
    NAMES SQLiteCpp
    PATHS 
    /usr/lib
    /usr/local/lib
    ${PROJECT_SOURCE_DIR}/libs/SQLiteCpp
    ${SQLITECPP_ROOT_DIR}
    PATH_SUFFIXES lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SQLiteCpp DEFAULT_MSG SQLITECPP_LIBRARY SQLITECPP_INCLUDE_DIR)

mark_as_advanced(SQLITECPP_INCLUDE_DIR SQLITECPP_LIBRARY)

set(SQLITECPP_LIBRARIES ${SQLITECPP_LIBRARY})
set(SQLITECPP_INCLUDE_DIRS ${SQLITECPP_INCLUDE_DIR}) 