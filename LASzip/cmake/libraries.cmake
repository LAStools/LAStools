# Build shared libraries by default.

option(LASZIP_BUILD_STATIC "Build LASzip as a static library" OFF)
if (LASZIP_BUILD_STATIC)
  set(LASZIP_LIB_TYPE "STATIC")
  set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX})
else ()
  set(LASZIP_LIB_TYPE "SHARED")
  if (WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_IMPORT_LIBRARY_SUFFIX})
  endif()
endif()
mark_as_advanced(LASZIP_BUILD_STATIC)
