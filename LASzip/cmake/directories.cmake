if(NOT ROOT_DIR)
    message(FATAL_ERROR "ROOT_DIR must be set in top-level CMakeLists.txt")
endif()
set(LASZIP_SRC_DIR ${ROOT_DIR}/src)
set(LASZIP_CMAKE_DIR ${ROOT_DIR}/cmake)
set(LASZIP_INCLUDE_DIR ${ROOT_DIR}/include)

