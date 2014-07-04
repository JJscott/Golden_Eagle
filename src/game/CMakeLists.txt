
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

add_definitions(-DGLEW_STATIC -DGLEW_MX)

target_link_libraries(game glew glfw ambition ${GLFW_LIBRARIES})

if(WIN32)
	target_link_libraries(game wsock32)
endif()