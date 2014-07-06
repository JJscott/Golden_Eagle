# add GTEST to include path
include_directories(${gtest_SOURCE_DIR}/include)

# get source files
# we could list these manually...
file(GLOB test_src "*.cpp")
file(GLOB test_hdr "*.hpp")

add_executable(test ${test_src} ${test_hdr})

set_target_properties(test
    PROPERTIES
	LINKER_LANGUAGE CXX
)

add_definitions(${AMBITION_DEFINITIONS})
target_link_libraries(test gtest_main ambition ${AMBITION_LIBRARIES})
