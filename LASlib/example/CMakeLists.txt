if (!MSVC)
	add_compile_options(-Wno-unused-result)
endif()

include_directories(../../LASzip/src ../inc)

set(TARGETS
	lasexample
	lasexample_write_only
	lasexample_add_rgb
	lasexample_simple_classification
	lasexample_write_only_with_extra_bytes)

foreach(TARGET ${TARGETS})
	add_executable(${TARGET} ${TARGET}.cpp)
	target_link_libraries(${TARGET} LASlib)
	set_target_properties(${TARGET} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endforeach(TARGET)
