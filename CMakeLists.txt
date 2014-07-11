project (GOLDEN)
cmake_minimum_required (VERSION 2.8.3)
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

# because nothing really works yet
set (GOLDEN_VERSION_MAJOR 0)
set (GOLDEN_VERSION_MINOR 0)
set (GOLDEN_VERSION_REV 0)

# OpenGL
include(FindOpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

# GLFW
add_subdirectory("${GOLDEN_SOURCE_DIR}/ext/glfw-3.0.3" "${GOLDEN_BINARY_DIR}/ext/glfw")
include_directories("${GOLDEN_SOURCE_DIR}/ext/glfw-3.0.3/include")

# GLEW
add_subdirectory("${GOLDEN_SOURCE_DIR}/ext/glew-1.10.0" "${GOLDEN_BINARY_DIR}/ext/glew")
include_directories("${GOLDEN_SOURCE_DIR}/ext/glew-1.10.0/include")

# CRYPTOPP
add_subdirectory("${GOLDEN_SOURCE_DIR}/ext/cryptopp-5.6.2" "${GOLDEN_BINARY_DIR}/ext/cryptopp")
include_directories("${GOLDEN_SOURCE_DIR}/ext/cryptopp-5.6.2/src")

# GTEST
add_subdirectory("${GOLDEN_SOURCE_DIR}/ext/gtest" "${GOLDEN_BINARY_DIR}/ext/gtest")

# compiler flags
#if(MSVC)
#	# Force to always compile with W4
#	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
#		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
#	else()
#		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
#   endif()
#elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
#	# Update if necessary
# 	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -std=c++11 -g")
#endif()

# compiler flags 
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	# Force to always compile with W4
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall -Werror=return-type -pedantic -std=c++11 -g")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall -Werror=return-type -pedantic -std=c++11 -g")
endif()

# output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# source
add_subdirectory(src)

# resources - this is a custom target with no build step;
# primarily to allow shader editing in IDEs
add_subdirectory(res)

# setup config file
configure_file (
	"${GOLDEN_SOURCE_DIR}/Config.hpp.in"
	"${GOLDEN_BINARY_DIR}/gen/Config.hpp"
)

# add generated config header to include path
include_directories("${GOLDEN_BINARY_DIR}/gen")

# set IDE display folders
SET_PROPERTY(TARGET glfw PROPERTY FOLDER "ext")
SET_PROPERTY(TARGET glew PROPERTY FOLDER "ext")
SET_PROPERTY(TARGET gtest PROPERTY FOLDER "ext")
SET_PROPERTY(TARGET gtest_main PROPERTY FOLDER "ext")
SET_PROPERTY(TARGET cryptopp PROPERTY FOLDER "ext")
SET_PROPERTY(TARGET ambition PROPERTY FOLDER "src")
SET_PROPERTY(TARGET test PROPERTY FOLDER "src")
SET_PROPERTY(TARGET game PROPERTY FOLDER "src")
SET_PROPERTY(TARGET server PROPERTY FOLDER "src")
SET_PROPERTY(TARGET res PROPERTY FOLDER "res")





















