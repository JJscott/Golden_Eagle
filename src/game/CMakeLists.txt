file(GLOB game_src "*.cpp")
file(GLOB game_header "*.hpp")
add_executable(game ${game_src} ${game_header})

set_target_properties( game
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

include_directories("${CMAKE_BINARY_DIR}/lib")

if(CMAKE_HOST_WIN32)
# make windows symlink to res
# TODO this requires vista / 7 / newer, identify older windows?
add_custom_command(TARGET game PRE_BUILD COMMAND cmd /S /C \" \"${CMAKE_SOURCE_DIR}/winshit/reslink.bat\" \"$<TARGET_FILE_DIR:game>/res\" \"${CMAKE_SOURCE_DIR}/res\" \")
elseif(CMAKE_HOST_UNIX)
# TODO make unix symlink to res
add_custom_command(TARGET game PRE_BUILD COMMAND ln -s ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:game>/res)
else()
# just copy the res dir instead
add_custom_command(TARGET game PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:game>/res)
endif()

target_link_libraries(game glfw common ${GLFW_LIBRARIES})
