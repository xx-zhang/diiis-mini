# FindGLEW.cmake
# Find GLEW library

find_path(GLEW_INCLUDE_DIR GL/glew.h
    PATHS 
    /usr/include
    /usr/local/include
    ${PROJECT_SOURCE_DIR}/libs/glew
    ${GLEW_ROOT_DIR}
    PATH_SUFFIXES include
)

find_library(GLEW_LIBRARY 
    NAMES GLEW glew
    PATHS 
    /usr/lib
    /usr/local/lib
    ${PROJECT_SOURCE_DIR}/libs/glew
    ${GLEW_ROOT_DIR}
    PATH_SUFFIXES lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GLEW DEFAULT_MSG GLEW_LIBRARY GLEW_INCLUDE_DIR)

mark_as_advanced(GLEW_INCLUDE_DIR GLEW_LIBRARY)

set(GLEW_LIBRARIES ${GLEW_LIBRARY})
set(GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIR}) 