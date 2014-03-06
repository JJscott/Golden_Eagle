file(GLOB game_src "*.cpp" "*.c")
file(GLOB game_header "*.hpp" "*.h")

add_executable(game ${game_src} ${game_header})

set_target_properties( game
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

include_directories("${CMAKE_BINARY_DIR}/lib")

add_definitions(-DGLEW_STATIC)
target_link_libraries(game glew glfw common ${GLFW_LIBRARIES})
