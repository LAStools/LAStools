##########################################################################
# These macros were taken from the Point Cloud Library (pointclouds.org) #
# and have been modified for LASZIP. License details follow.               #
##########################################################################
# Software License Agreement (BSD License)                               #
#                                                                        #
# Point Cloud Library (PCL) - www.pointclouds.org                        #
# Copyright (c) 2009-2012, Willow Garage, Inc.                           #
# Copyright (c) 2012-, Open Perception, Inc.                             #
# Copyright (c) XXX, respective authors.                                 #
#                                                                        #
# All rights reserved.                                                   #
#                                                                        #
# Redistribution and use in source and binary forms, with or without     #
# modification, are permitted provided that the following conditions     #
# are met:                                                               #
#                                                                        #
#  * Redistributions of source code must retain the above copyright      #
#    notice, this list of conditions and the following disclaimer.       #
#  * Redistributions in binary form must reproduce the above             #
#    copyright notice, this list of conditions and the following         #
#    disclaimer in the documentation and/or other materials provided     #
#    with the distribution.                                              #
#  * Neither the name of the copyright holder(s) nor the names of its    #
#    contributors may be used to endorse or promote products derived     #
#    from this software without specific prior written permission.       #
#                                                                        #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    #
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      #
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS      #
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE         #
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,    #
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,   #
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;       #
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER       #
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT     #
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN      #
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE        #
# POSSIBILITY OF SUCH DAMAGE.                                            #
##########################################################################


###############################################################################
# Add a set of include files to install.
# _component The part of LASZIP that the install files belong to.
# _subdir The sub-directory for these include files.
# ARGN The include files.
macro(LASZIP_ADD_INCLUDES _subdir)
    install(FILES ${ARGN} DESTINATION ${LASZIP_INCLUDE_INSTALL_DIR}/${_subdir})
endmacro(LASZIP_ADD_INCLUDES)


###############################################################################
# Add a library target.
# _name The library name.
# _component The part of LASZIP that this library belongs to.
# ARGN The source files for the library.
macro(LASZIP_ADD_LIBRARY _name)
    add_library(${_name} ${LASZIP_LIB_TYPE} ${ARGN})
    set_target_properties(
        ${_name} PROPERTIES
        VERSION ${LASZIP_SO_VERSION}
        SOVERSION ${LASZIP_COMPATIBILITY_VERSION}
        CLEAN_DIRECT_OUTPUT 1
        FOLDER Libraries
    )

    install(TARGETS ${_name}
        EXPORT LASZIPTargets
        RUNTIME DESTINATION ${LASZIP_BIN_INSTALL_DIR}
        LIBRARY DESTINATION ${LASZIP_LIB_INSTALL_DIR}
        ARCHIVE DESTINATION ${LASZIP_LIB_INSTALL_DIR})
    if (APPLE)
        set_target_properties(${_name} PROPERTIES INSTALL_NAME_DIR
            "@executable_path/../lib")
    endif()
endmacro(LASZIP_ADD_LIBRARY)

###############################################################################
# Add an executable target.
# _name The executable name.
# _component The part of LASZIP that this library belongs to.
# ARGN the source files for the library.
macro(LASZIP_ADD_EXECUTABLE _name)
    add_executable(${_name} ${ARGN})

    set(LASZIP_EXECUTABLES ${LASZIP_EXECUTABLES} ${_name})
    install(TARGETS ${_name}
        EXPORT LASZIPTargets
        RUNTIME DESTINATION ${LASZIP_BIN_INSTALL_DIR})
endmacro(LASZIP_ADD_EXECUTABLE)


###############################################################################
# Add a test target.
# _name The driver name.
# ARGN :
#    FILES the source files for the test
#    LINK_WITH link test executable with libraries
macro(LASZIP_ADD_TEST _name)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs FILES LINK_WITH)
    cmake_parse_arguments(LASZIP_ADD_TEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    include_directories(${PROJECT_SOURCE_DIR}/test/unit)
    include_directories(${PROJECT_BINARY_DIR}/test/unit)
    set(common_srcs
        ${PROJECT_SOURCE_DIR}/test/unit/Support.cpp
        ${PROJECT_SOURCE_DIR}/test/unit/TestConfig.cpp
    )
    if (WIN32)
        list(APPEND ${LASZIP_ADD_TEST_FILES} ${LASZIP_TARGET_OBJECTS})
        add_definitions("-DLASZIP_DLL_EXPORT=1")
    endif()
    add_executable(${_name} ${LASZIP_ADD_TEST_FILES} ${common_srcs})
    set_target_properties(${_name} PROPERTIES COMPILE_DEFINITIONS LASZIP_DLL_IMPORT)
    set_property(TARGET ${_name} PROPERTY FOLDER "Tests")
    target_link_libraries(${_name} ${LASZIP_BASE_LIB_NAME} gtest
        ${LASZIP_ADD_TEST_LINK_WITH})
    add_test(NAME ${_name} COMMAND "${PROJECT_BINARY_DIR}/bin/${_name}" WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/..")
    set_property(TEST ${_name} PROPERTY ENVIRONMENT
      # Ensure plugins are loaded from build dir
      # https://github.com/LASZIP/LASZIP/issues/840
      "LASZIP_DRIVER_PATH=${PROJECT_BINARY_DIR}/lib"
    )
endmacro(LASZIP_ADD_TEST)


###############################################################################
# Get the operating system information. Generally, CMake does a good job of
# this. Sometimes, though, it doesn't give enough information. This macro will
# distinguish between the UNIX variants. Otherwise, use the CMake variables
# such as WIN32 and APPLE and CYGWIN.
# Sets OS_IS_64BIT if the operating system is 64-bit.
# Sets LINUX if the operating system is Linux.
macro(GET_OS_INFO)
    string(REGEX MATCH "Linux" OS_IS_LINUX ${CMAKE_SYSTEM_NAME})
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(OS_IS_64BIT TRUE)
    else(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(OS_IS_64BIT FALSE)
    endif(CMAKE_SIZEOF_VOID_P EQUAL 8)
endmacro(GET_OS_INFO)

###############################################################################
# Pull the component parts out of the version number.
macro(DISSECT_VERSION)
    # Find version components
    string(REGEX REPLACE "^([0-9]+).*" "\\1"
        LASZIP_API_VERSION_MAJOR "${LASZIP_API_VERSION_STRING}")
    string(REGEX REPLACE "^[0-9]+\\.([0-9]+).*" "\\1"
        LASZIP_API_VERSION_MINOR "${LASZIP_API_VERSION_STRING}")
    string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1"
        LASZIP_API_VERSION_PATCH "${LASZIP_API_VERSION_STRING}")
    if (NOT LASZIP_API_VERSION_MINOR)
        set(LASZIP_API_VERSION_MINOR "0")
    endif()
    if (NOT LASZIP_API_VERSION_PATCH)
        set(LASZIP_API_VERSION_PATCH "0")
    endif()
endmacro(DISSECT_VERSION)

###############################################################################
# Set the destination directories for installing stuff.
# Sets LASZIP_LIB_INSTALL_DIR. Install libraries here.
# Sets LASZIP_BIN_INSTALL_DIR. Install binaries here.
# Sets LASZIP_INCLUDE_INSTALL_DIR. Install include files here, preferably in a
# subdirectory named after the library in question (e.g.
# "registration/blorgle.h")
macro(SET_INSTALL_DIRS)
  string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)
  if (NOT DEFINED LASZIP_LIB_INSTALL_DIR)
      if (DEFINED CMAKE_INSTALL_LIBDIR)
          set(LASZIP_LIB_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}")
      else()
          set(LASZIP_LIB_INSTALL_DIR "lib")
      endif()
  endif ()
    set(LASZIP_INCLUDE_INSTALL_ROOT "include/")
    set(LASZIP_INCLUDE_INSTALL_DIR
        "${LASZIP_INCLUDE_INSTALL_ROOT}")
    set(LASZIP_DOC_INCLUDE_DIR
        "share/doc/${PROJECT_NAME_LOWER}-${LASZIP_VERSION_MAJOR}.${LASZIP_VERSION_MINOR}")
    set(LASZIP_BIN_INSTALL_DIR "bin")
    set(LASZIP_PLUGIN_INSTALL_DIR "share/pdal/plugins")
    if(WIN32)
        set(LASZIPCONFIG_INSTALL_DIR "cmake")
    else(WIN32)
        set(LASZIPCONFIG_INSTALL_DIR
            "share/${PROJECT_NAME_LOWER}-${LASZIP_VERSION_MAJOR}.${LASZIP_VERSION_MINOR}")
    endif(WIN32)
endmacro(SET_INSTALL_DIRS)
