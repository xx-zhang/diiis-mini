# FindBoost.cmake
# Find Boost library

find_path(BOOST_INCLUDE_DIR boost/version.hpp
    PATHS 
    /usr/include
    /usr/local/include
    ${PROJECT_SOURCE_DIR}/libs/boost
    ${BOOST_ROOT_DIR}
    PATH_SUFFIXES include
)

find_library(BOOST_SYSTEM_LIBRARY 
    NAMES boost_system
    PATHS 
    /usr/lib
    /usr/local/lib
    ${PROJECT_SOURCE_DIR}/libs/boost
    ${BOOST_ROOT_DIR}
    PATH_SUFFIXES lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Boost DEFAULT_MSG BOOST_SYSTEM_LIBRARY BOOST_INCLUDE_DIR)

mark_as_advanced(BOOST_INCLUDE_DIR BOOST_SYSTEM_LIBRARY)

set(BOOST_LIBRARIES ${BOOST_SYSTEM_LIBRARY})
set(BOOST_INCLUDE_DIRS ${BOOST_INCLUDE_DIR}) 