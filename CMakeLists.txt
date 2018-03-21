cmake_minimum_required(VERSION 2.8)

project(LAStools)

SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMake")

function(prepend_path var prefix)
   set(listVar "")
   foreach(f ${ARGN})
      list(APPEND listVar "${prefix}/${f}")
   endforeach(f)
   set(${var} "${listVar}" PARENT_SCOPE)
endfunction(prepend_path)

set(LASZIP_SRCS arithmeticdecoder.cpp lasindex.cpp lasreaditemcompressed_v2.cpp
                laswriteitemcompressed_v2.cpp laszipper.cpp arithmeticencoder.cpp
                lasinterval.cpp lasreadpoint.cpp laswritepoint.cpp arithmeticmodel.cpp
                lasquadtree.cpp lasunzipper.cpp laszip.cpp integercompressor.cpp
                lasreaditemcompressed_v1.cpp laswriteitemcompressed_v1.cpp)# laszip_dll.cpp)
set(LASZIP_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/LASzip/src/" CACHE PATH "laszip include dir")
set(LASZIP_DLL_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/LASzip/dll/" CACHE PATH "laszip dllinclude dir")
set(LASZIP_INCLUDE_DIRS ${LASZIP_INCLUDE_DIR} ${LASZIP_DLL_INCLUDE_DIR})
include_directories(${LASZIP_INCLUDE_DIRS})
prepend_path(LASZIP_SRCS "${PROJECT_SOURCE_DIR}/LASzip/src/" ${LASZIP_SRCS})
message(STATUS "laszip srcs ${LASZIP_SRCS}")

set(LASLIB_SRCS lasreader_bil.cpp lasreader_shp.cpp lastransform.cpp laswriter_bin.cpp
                laswritercompatible.cpp fopen_compressed.cpp  lasreader_bin.cpp lasreader_txt.cpp
                lasutility.cpp laswriter_las.cpp lasfilter.cpp lasreader_dtm.cpp   lasreaderbuffered.cpp  laswaveform13reader.cpp  laswriter_qfit.cpp
                lasreader.cpp lasreader_las.cpp lasreadermerged.cpp laswaveform13writer.cpp  laswriter_txt.cpp lasreader_asc.cpp lasreader_qfit.cpp lasreaderpipeon.cpp    laswriter.cpp laswriter_wrl.cpp)
set(LASLIB_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/LASlib/inc/" CACHE PATH "laslib include dir")
include_directories(${LASLIB_INCLUDE_DIRS})
prepend_path(LASLIB_SRCS "${PROJECT_SOURCE_DIR}/LASlib/src/" ${LASLIB_SRCS})
message(STATUS "laslib srcs ${LASLIB_SRCS}")

add_library(laslib STATIC ${LASZIP_SRCS} ${LASLIB_SRCS})

set(LASZIP_HEADERS arithmeticdecoder.hpp     bytestreaminout_file.hpp   lasindex.hpp                  lasreaditemraw.hpp             laszip.hpp
                   arithmeticencoder.hpp     bytestreamout.hpp          lasinterval.hpp               lasreadpoint.hpp                laszip_common_v1.hpp
                   arithmeticmodel.hpp       bytestreamout_array.hpp    laspoint.hpp                  lasunzipper.hpp                laszip_common_v2.hpp
                   bytestreamin.hpp          bytestreamout_file.hpp     lasquadtree.hpp               laswriteitem.hpp               laszipper.hpp
                   bytestreamin_array.hpp    bytestreamout_nil.hpp      lasquantizer.hpp              laswriteitemcompressed_v1.hpp  mydefs.hpp
                   bytestreamin_file.hpp     bytestreamout_ostream.hpp  lasreaditem.hpp               laswriteitemcompressed_v2.hpp
                   bytestreamin_istream.hpp  integercompressor.hpp      lasreaditemcompressed_v1.hpp  laswriteitemraw.hpp
                   bytestreaminout.hpp       lasattributer.hpp          lasreaditemcompressed_v2.hpp  laswritepoint.hpp)
prepend_path(LASZIP_HEADERS "${PROJECT_SOURCE_DIR}/LASzip/src/" ${LASZIP_HEADERS})

set(LASLIB_HEADERS lasdefinitions.hpp  lasreader_bin.hpp   lasreader_txt.hpp      lasutility.hpp           laswriter_las.hpp
                   lasfilter.hpp       lasreader_dtm.hpp   lasreaderbuffered.hpp  laswaveform13reader.hpp  laswriter_qfit.hpp
                   lasreader.hpp       lasreader_las.hpp   lasreadermerged.hpp    laswaveform13writer.hpp  laswriter_txt.hpp
                   lasreader_asc.hpp   lasreader_qfit.hpp  lasreaderpipeon.hpp    laswriter.hpp            laswriter_wrl.hpp
                   lasreader_bil.hpp   lasreader_shp.hpp   lastransform.hpp       laswriter_bin.hpp        laswritercompatible.hpp)
prepend_path(LASLIB_HEADERS "${PROJECT_SOURCE_DIR}/LASlib/inc/" ${LASLIB_HEADERS})

INSTALL(TARGETS laslib
        DESTINATION "${CMAKE_BINARY_DIR}/lib"
        )
INSTALL(FILES ${LASZIP_HEADERS}
        DESTINATION "${CMAKE_BINARY_DIR}/laszip/include"
        )
INSTALL(FILES ${LASLIB_HEADERS}
        DESTINATION "${CMAKE_BINARY_DIR}/laslib/include"
        )