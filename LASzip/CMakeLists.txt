cmake_minimum_required(VERSION 2.8.11)
set(CMAKE_SUPPRESS_REGENERATION true)

project(LASzip CXX C)
string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)

set(ROOT_DIR "${PROJECT_SOURCE_DIR}")

# the next line is the ONLY place in the entire laszip system where
# the version info is hard-coded
set(LASZIP_API_VERSION_STRING "3.4.1" CACHE STRING "LASzip version" FORCE)

include (CheckIncludeFileCXX)
include(${ROOT_DIR}/cmake/common.cmake NO_POLICY_SCOPE)
include(${ROOT_DIR}/cmake/cpack.cmake NO_POLICY_SCOPE)

#------------------------------------------------------------------------------
# internal cmake settings
#------------------------------------------------------------------------------

set(CMAKE_COLOR_MAKEFILE ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
include(FeatureSummary)

# Allow advanced users to generate Makefiles printing detailed commands
mark_as_advanced(CMAKE_VERBOSE_MAKEFILE)

#------------------------------------------------------------------------------
# LASzip general settings
#------------------------------------------------------------------------------


#
# Extract the pieces of the version info from the version string above.
DISSECT_VERSION()
GET_OS_INFO()
SET_INSTALL_DIRS()


set(LASZIP_API_VERSION ${LASZIP_API_VERSION_MAJOR}.${LASZIP_API_VERSION_MINOR}.${LASZIP_API_VERSION_PATCH})

# libtool SO version naming
# 8.0.0 for 3.2.1
# 9.0.0 for 4.0+
set(LASZIP_SO_VERSION "8.0.4")
set(LASZIP_COMPATIBILITY_VERSION 8)

check_include_file_cxx ("unordered_map" HAVE_UNORDERED_MAP)

###############################################################################
# Main CMake configuration file for laszip
#
# Author: Mateusz Loskot <mateusz@loskot.net>
#
#
###############################################################################
# laszip general settings

set(LASZIP_API_VERSION_H
    ${CMAKE_CURRENT_BINARY_DIR}/dll/laszip_api_version.h)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/laszip_api_version.h.in
    ${LASZIP_API_VERSION_H})

set(LASZIP_BASE_LIB_NAME laszip)
set(LASZIP_API_LIB_NAME laszip_api)

if(WIN32)
    set(LASZIP_BASE_LIB_NAME "${LASZIP_BASE_LIB_NAME}${LASZIP_API_VERSION_MAJOR}")
    set(LASZIP_API_LIB_NAME "${LASZIP_API_LIB_NAME}${LASZIP_API_VERSION_MAJOR}")
endif()


set(LASZIP_OUTPUT_LIB_DIR "${LASZIP_BINARY_DIR}/${LASZIP_LIB_INSTALL_DIR}")
set(LASZIP_OUTPUT_BIN_DIR "${LASZIP_BINARY_DIR}/${LASZIP_BIN_INSTALL_DIR}")
set(LASZIP_PLUGIN_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}/${LASZIP_LIB_INSTALL_DIR}")
file(MAKE_DIRECTORY "${LASZIP_OUTPUT_LIB_DIR}")
file(MAKE_DIRECTORY "${LASZIP_OUTPUT_BIN_DIR}")

# per http://www.cmake.org/Wiki/CMake_RPATH_handling
SET(CMAKE_SKIP_BUILD_RPATH FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
IF (APPLE)
    SET(MACOSX_RPATH ON)
endif()
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
    "${CMAKE_INSTALL_PREFIX}/${LASZIP_LIB_INSTALL_DIR}" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
    SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${LASZIP_LIB_INSTALL_DIR}")
ENDIF("${isSystemDir}" STREQUAL "-1")

# wipe lib/ drectory on clean. It will have plugins that could be out of date
# in the next build
set_directory_properties(PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${LASZIP_OUTPUT_LIB_DIR}/*")

if(WIN32)
  add_definitions("-DLASZIP_DLL_EXPORT=1")
  foreach(config ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${config} CONFIG)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG} "${LASZIP_OUTPUT_LIB_DIR}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG} "${LASZIP_OUTPUT_BIN_DIR}")
    # ---[ Windows requires DLLs (shared libraries) to be installed in the same directory as executables
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG} "${LASZIP_OUTPUT_BIN_DIR}")
  endforeach(config)
else(WIN32)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${LASZIP_OUTPUT_LIB_DIR}")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${LASZIP_OUTPUT_BIN_DIR}")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${LASZIP_OUTPUT_LIB_DIR}")
endif(WIN32)

set(LASZIP_HEADERS_DIR "${PROJECT_SOURCE_DIR}/dll")

add_subdirectory(src)
add_subdirectory(dll)

#
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
