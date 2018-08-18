add_compile_options(-Wno-deprecated -Wno-write-strings -Wno-unused-result)
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
)
set(STANDALONE_TARGETS
	las2txt
	lasdiff
	lasindex
)
set(ALL_TARGETS ${GEOPROJECTION_TARGETS} ${STANDALONE_TARGETS})

add_library(geoprojectionconverter OBJECT geoprojectionconverter.cpp)

foreach(TARGET ${GEOPROJECTION_TARGETS})
	add_executable(${TARGET} ${TARGET}.cpp $<TARGET_OBJECTS:geoprojectionconverter>)
endforeach(TARGET)

foreach(TARGET ${STANDALONE_TARGETS})
	add_executable(${TARGET} ${TARGET}.cpp)
endforeach(TARGET)

foreach(TARGET ${ALL_TARGETS})
	target_link_libraries(${TARGET} las)
	set_target_properties(${TARGET} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../bin)
	install(TARGETS ${TARGET} RUNTIME DESTINATION bin)
endforeach(TARGET)
