file(GLOB server_src "*.cpp" "*.c")
file(GLOB server_header "*.hpp" "*.h")

add_executable(server ${server_src} ${server_header})

set_target_properties(server
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

include_directories("${CMAKE_BINARY_DIR}/lib")

# add_custom_command(TARGET server PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:server>/res)

target_link_libraries(server common ${GLFW_LIBRARIES})
