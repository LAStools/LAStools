include("${CMAKE_CURRENT_LIST_DIR}/LASlibTargets.cmake")
add_library(LAStools::LASlib INTERFACE IMPORTED)
target_link_libraries(LAStools::LASlib INTERFACE LASlib)

get_target_property(LASlib_INCLUDE_DIRS LASlib INTERFACE_INCLUDE_DIRECTORIES)
set(LASlib_LIBRARIES LAStools::LASlib)

set(LASlib_FOUND true)
