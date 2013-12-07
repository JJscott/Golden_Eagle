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

add_subdirectory("./common")
add_subdirectory("./server")
add_subdirectory("./gamesrv")
add_subdirectory("./game")

