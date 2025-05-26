# FindImGui.cmake
# Find ImGui library

find_path(IMGUI_INCLUDE_DIR imgui.h
    PATHS 
    ${PROJECT_SOURCE_DIR}/libs/imgui
    ${IMGUI_ROOT_DIR}
    PATH_SUFFIXES include
)

find_library(IMGUI_LIBRARY 
    NAMES imgui
    PATHS 
    ${PROJECT_SOURCE_DIR}/libs/imgui
    ${IMGUI_ROOT_DIR}
    PATH_SUFFIXES lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ImGui DEFAULT_MSG IMGUI_LIBRARY IMGUI_INCLUDE_DIR)

mark_as_advanced(IMGUI_INCLUDE_DIR IMGUI_LIBRARY)

set(IMGUI_LIBRARIES ${IMGUI_LIBRARY})
set(IMGUI_INCLUDE_DIRS ${IMGUI_INCLUDE_DIR}) 