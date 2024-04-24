# Install script for directory: /work/LAStools_full/LAStools/LASlib/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/LASlib" TYPE FILE FILES
    "/work/LAStools_full/LAStools/LASlib/inc/lascopc.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/lasdefinitions.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/lasfilter.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/lasignore.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/laskdtree.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/lasprogress.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/lasreader.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/lasreader_asc.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/lasreader_bil.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/lasreader_bin.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/lasreader_dtm.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/lasreader_las.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/lasreader_ply.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/lasreader_qfit.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/lasreader_shp.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/lasreader_txt.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/lasreaderbuffered.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/lasreadermerged.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/lasreaderpipeon.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/lasreaderstored.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/lastransform.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/lasutility.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/lasvlr.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/lasvlrpayload.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/laswaveform13reader.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/laswaveform13writer.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/laswriter.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/laswriter_bin.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/laswriter_las.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/laswriter_qfit.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/laswriter_txt.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/laswriter_wrl.hpp"
    "/work/LAStools_full/LAStools/LASlib/inc/laswritercompatible.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/arithmeticdecoder.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/arithmeticencoder.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/arithmeticmodel.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/bytestreamin.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/bytestreamin_array.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/bytestreamin_file.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/bytestreamin_istream.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/bytestreaminout.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/bytestreaminout_file.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/bytestreamout.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/bytestreamout_array.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/bytestreamout_file.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/bytestreamout_nil.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/bytestreamout_ostream.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/endian.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/integercompressor.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/lasattributer.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/lasindex.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/lasinterval.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/lasmessage.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/laspoint.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/lasquadtree.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/lasquantizer.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/lasreaditem.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/lasreaditemcompressed_v1.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/lasreaditemcompressed_v2.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/lasreaditemcompressed_v3.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/lasreaditemcompressed_v4.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/lasreaditemraw.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/lasreadpoint.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/lasunzipper.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/laswriteitem.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/laswriteitemcompressed_v1.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/laswriteitemcompressed_v2.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/laswriteitemcompressed_v3.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/laswriteitemcompressed_v4.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/laswriteitemraw.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/laswritepoint.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/laszip.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/laszip_common_v1.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/laszip_common_v2.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/laszip_common_v3.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/laszip_decompress_selective_v3.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/laszipper.hpp"
    "/work/LAStools_full/LAStools/LASzip/src/mydefs.hpp"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/LASlib" TYPE STATIC_LIBRARY FILES "/work/LAStools_full/LAStools/LASlib/lib/libLASlib.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/LASlib/laslib-targets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/LASlib/laslib-targets.cmake"
         "/work/LAStools_full/LAStools/LASlib/src/CMakeFiles/Export/c55326e5cb745217e14af8383b523273/laslib-targets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/LASlib/laslib-targets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/LASlib/laslib-targets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/LASlib" TYPE FILE FILES "/work/LAStools_full/LAStools/LASlib/src/CMakeFiles/Export/c55326e5cb745217e14af8383b523273/laslib-targets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/LASlib" TYPE FILE FILES "/work/LAStools_full/LAStools/LASlib/src/CMakeFiles/Export/c55326e5cb745217e14af8383b523273/laslib-targets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/LASlib" TYPE FILE FILES "/work/LAStools_full/LAStools/LASlib/src/laslib-config.cmake")
endif()

