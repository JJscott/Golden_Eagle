
# get source files
# we could list these manually...
file(GLOB server_src "*.cpp" "*.c")
file(GLOB server_hdr "*.hpp" "*.h")

add_executable(server ${server_src} ${server_hdr})

set_target_properties(
	server
    PROPERTIES
    LINKER_LANGUAGE CXX
)

# wtf was this for? -ben
# add_custom_command(TARGET server PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:gamesrv>/res)

target_link_libraries(server ambition)

if(WIN32)
	target_link_libraries(server wsock32)
endif()
