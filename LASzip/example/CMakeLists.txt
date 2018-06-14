include_directories(../src)

set(LASZIPPERTEST_SRC
	laszippertest.cpp
	../src/laszip.cpp
	../src/laszipper.cpp
	../src/lasunzipper.cpp
	../src/lasreadpoint.cpp
	../src/lasreaditemcompressed_v1.cpp
	../src/lasreaditemcompressed_v2.cpp
	../src/lasreaditemcompressed_v3.cpp
	../src/lasreaditemcompressed_v4.cpp
	../src/laswritepoint.cpp
	../src/laswriteitemcompressed_v1.cpp
	../src/laswriteitemcompressed_v2.cpp
	../src/laswriteitemcompressed_v3.cpp
	../src/laswriteitemcompressed_v4.cpp
	../src/integercompressor.cpp
	../src/arithmeticdecoder.cpp
	../src/arithmeticencoder.cpp
	../src/arithmeticmodel.cpp
)
add_executable(laszippertest ${LASZIPPERTEST_SRC})
set_target_properties(laszippertest PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
