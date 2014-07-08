
# get source files
# we could list these manually...
file(GLOB nogl_game_src "*.cpp" "*.c")
file(GLOB nogl_game_hdr "*.hpp" "*.h")

add_executable(nogl ${nogl_game_src} ${nogl_game_hdr})

set_target_properties(
	nogl
	PROPERTIES
	LINKER_LANGUAGE CXX
)

add_definitions(${AMBITION_DEFINITIONS})
target_link_libraries(nogl ambition ${AMBITION_LIBRARIES})
