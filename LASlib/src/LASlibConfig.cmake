include("${CMAKE_CURRENT_LIST_DIR}/LASlibTargets.cmake")
add_library(LASlib ALIAS LAStools::LASlib) # add legacy alias target name to keep compatibility

get_target_property(LASlib_INCLUDE_DIRS LAStools::LASlib INTERFACE_INCLUDE_DIRECTORIES)
set(LASlib_LIBRARIES LAStools::LASlib)

set(LASlib_FOUND true)
