if (!MSVC)
add_compile_options(-Wno-deprecated -Wno-write-strings -Wno-unused-result)
endif()
add_definitions(-DNDEBUG )

include_directories(../LASzip/src)
include_directories(../LASlib/inc)

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
	add_executable(${TARGET} ${TARGET}.cpp geoprojectionconverter.cpp)
endforeach(TARGET)

foreach(TARGET ${STANDALONE_TARGETS})
	add_executable(${TARGET} ${TARGET}.cpp)
endforeach(TARGET)

foreach(TARGET ${ALL_TARGETS})
	set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 14)
	target_link_libraries(${TARGET} LASlib)
	set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../bin64)
	set_target_properties(${TARGET} PROPERTIES OUTPUT_NAME ${TARGET}64)
	install(TARGETS ${TARGET} RUNTIME DESTINATION bin)
endforeach(TARGET)
