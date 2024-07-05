###############################################################################
#
# tools/CMakeLists.txt controls building of laszip utilities 
#
# Copyright (c) 2009 Mateusz Loskot <mateusz@loskot.net>
#
###############################################################################

include_directories(
    .
    ../include
    ../include/laszip)


set(APPS_CPP_DEPENDENCIES
    ${LASZIP_LIB_NAME})
    

set(LASZIPPERTEST laszippertest)


set(LASZIP_UTILITIES
    ${LASZIPPERTEST}
    )
    
if(WIN32)
if(NOT BUILD_STATIC)
    add_definitions("-DLASZIP_DLL_IMPORTS=1")
endif()
endif()


# TODO: Experimental and requires testing --mloskot
# Generate user-specific settings for Visual Studio project
set(VCPROJ_USER_REMOTE_MACHINE_DEBUG ${MACHINE_NAME})
set(VCPROJ_USER_ENVIRONMENT_DEBUG "${ENVIRONMENT_PATH}")

if(MSVC)
    foreach(utility ${LASZIP_UTILITIES})
        set(USER_FILE ${utility}.vcproj.$ENV{USERDOMAIN}.$ENV{USERNAME}.user)
        set(OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/${USER_FILE})
        message(STATUS "Generating ${CMAKE_GENERATOR} user-specific settings in ${USER_FILE}")
        configure_file(${CMAKE_SOURCE_DIR}/cmake/laszip.vcproj.user.template ${OUTPUT_PATH} @ONLY)
    endforeach()
endif()


if(LASZIPPERTEST)
    add_executable(${LASZIPPERTEST} laszippertest.cpp)
    target_link_libraries(${LASZIPPERTEST} ${APPS_CPP_DEPENDENCIES} )
    if (EMSCRIPTEN)
        SET_TARGET_PROPERTIES(${LASZIPPERTEST} PROPERTIES SUFFIX .js)
    endif()
endif()


if(UNIX)
    # Autoconf compatibility variables to use the same script source.
    set(prefix ${CMAKE_INSTALL_PREFIX})
    set(exec_prefix ${CMAKE_INSTALL_PREFIX}/bin)
    set(libdir ${CMAKE_INSTALL_PREFIX}/lib)

    GET_DIRECTORY_PROPERTY(LASZIP_DEFINITIONS DIRECTORY ${laszip_SOURCE_DIR}/ COMPILE_DEFINITIONS)   

    set(LASZIP_CONFIG_DEFINITIONS "")
    foreach(definition ${LASZIP_DEFINITIONS})
        set(LASZIP_CONFIG_DEFINITIONS "${LASZIP_CONFIG_DEFINITIONS} -D${definition}")
    endforeach()

 
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/laszip-config.in
      ${CMAKE_CURRENT_BINARY_DIR}/laszip-config @ONLY)
  
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/laszip-config
      DESTINATION bin/
      PERMISSIONS
      OWNER_READ OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

    #pkgconfig
    set(PKGCFG_PREFIX "${CMAKE_INSTALL_PREFIX}")
    set(PKGCFG_INC_DIR "${CMAKE_INSTALL_PREFIX}/${LIBLAS_INCLUDE_SUBDIR}")
    set(PKGCFG_LIB_DIR "${CMAKE_INSTALL_PREFIX}/${LIBLAS_LIB_SUBDIR}")
    set(PKGCFG_REQUIRES  "")
    set(PKGCFG_VERSION ${VERSION})
    set(PKGCFG_LINK_FLAGS "-llaszip")
    set(PKGCFG_COMPILE_FLAGS "$CMAKE_CXX_FLAGS")
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/laszip.pc.in
      ${CMAKE_CURRENT_BINARY_DIR}/laszip.pc @ONLY)

  
endif(UNIX)
###############################################################################
# Targets installation

install(TARGETS ${LASZIP_UTILITIES}
    RUNTIME DESTINATION ${LASZIP_BIN_DIR}
    LIBRARY DESTINATION ${LASZIP_LIB_DIR}
    ARCHIVE DESTINATION ${LASZIP_LIB_DIR})
