include_directories(
    ${LASZIP_INCLUDE_DIR}
    ${PROJECT_BINARY_DIR}/dll
)
if (WIN32)
    include (${CMAKE_CURRENT_LIST_DIR}/win32_compiler_options.cmake)
else()
    include (${CMAKE_CURRENT_LIST_DIR}/unix_compiler_options.cmake)
endif()

