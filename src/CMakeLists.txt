if(CMAKE_GNU_COMPILER_ID OR CMAKE_Clang_COMPILER_ID)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-format-security -Wno-format-truncation")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-format-security -Wno-format-truncation -std=c++17")
endif()
if (NOT(MSVC))
  add_compile_options(-Wno-deprecated -Wno-write-strings -Wno-unused-result)
endif()

add_definitions(-DNDEBUG )

include_directories(../LASzip/include/laszip)
include_directories(../LASzip/src)
include_directories(../LASlib/inc)
include_directories(.)

set(GEOPROJECTION_TARGETS
  laszip
  lasinfo
  lasprecision
  txt2las
  las2las
  lasmerge
  lascopcindex
)
set(STANDALONE_TARGETS
  las2txt
  lasdiff
  lasindex
)
set(ALL_TARGETS ${GEOPROJECTION_TARGETS} ${STANDALONE_TARGETS})

foreach(TARGET ${GEOPROJECTION_TARGETS})
  add_executable(${TARGET} ${TARGET}.cpp wktparser.cpp geoprojectionconverter.cpp proj_loader.cpp proj_wrapper.cpp)
endforeach(TARGET)

foreach(TARGET ${STANDALONE_TARGETS})
  add_executable(${TARGET} ${TARGET}.cpp proj_loader.cpp proj_wrapper.cpp)
endforeach(TARGET)

foreach(TARGET ${ALL_TARGETS})
  set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 17)
  target_link_libraries(${TARGET} LASlib ${CMAKE_DL_LIBS})
  set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../bin64)
  set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME ${TARGET}64)
  install(TARGETS ${TARGET} RUNTIME DESTINATION bin)
endforeach(TARGET)
