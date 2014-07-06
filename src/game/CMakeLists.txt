
# get source files
# we could list these manually...
file(GLOB game_src "*.cpp" "*.c")
file(GLOB game_hdr "*.hpp" "*.h")

add_executable(game ${game_src} ${game_hdr})

set_target_properties(
	game
    PROPERTIES
    LINKER_LANGUAGE CXX
)

add_definitions(${AMBITION_DEFINITIONS})
target_link_libraries(game ambition ${AMBITION_LIBRARIES})
