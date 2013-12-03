configure_file (
	"${PROJECT_SOURCE_DIR}/src/Config.hpp.in"
	"${PROJECT_BINARY_DIR}/Config.hpp"
)

include(FindOpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

add_subdirectory(${CMAKE_SOURCE_DIR}/ext/glfw-3.0.3 "${CMAKE_BINARY_DIR}/bin/cmake")
include_directories(${CMAKE_SOURCE_DIR}/ext/glfw-3.0.3/include)

if(MSVC)
  # Force to always compile with W4
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic -std=c++11")
endif()

include_directories("${CMAKE_SOURCE_DIR}/src")

source_group(common FILES "common/*.cpp" "common/*.hpp" "common/*.h" "common/*.c")

file(GLOB game_src "game/*.cpp")
file(GLOB game_header "game/*.hpp")
add_executable(game ${game_src} ${game_header} ${common})
set_target_properties( game
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_custom_command(TARGET game PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:game>/res)
target_link_libraries(game glfw ${GLFW_LIBRARIES})

file(GLOB server_src "server/*.cpp")
file(GLOB server_header "server/*.hpp")
add_executable(server ${server_src} ${server_header} ${common})
set_target_properties( server
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_custom_command(TARGET server PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res $<TARGET_FILE_DIR:server>/res)
target_link_libraries(server glfw ${GLFW_LIBRARIES})
