file(GLOB game_src "*.cpp")
file(GLOB game_header "*.hpp")
add_executable(game ${game_src} ${game_header})

set_target_properties( game
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_custom_command(TARGET game PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:game>/res)
target_link_libraries(game glfw common ${GLFW_LIBRARIES})
