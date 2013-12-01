configure_file (
	"${PROJECT_SOURCE_DIR}/src/Config.hpp.in"
	"${PROJECT_BINARY_DIR}/Config.hpp"
)

file(GLOB ge_src "*.cpp")

add_executable(Golden ${ge_src})
set_target_properties( Golden
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_custom_command(TARGET Golden PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:Golden>/res)

include_directories("${PROJECT_BINARY_DIR}")

include(FindOpenGL REQUIRED)
include(FindGLEW REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_DIR})

target_link_libraries(Golden glfw3 ${GLFW_LIBRARIES})
target_link_libraries(Golden GLEW ${GLEW_LIBRARIES})

