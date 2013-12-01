cmake_minimum_required (VERSION 2.6)
project (Golden)

set (Golden_VERSION_MAJOR 0)
set (Golden_VERSION_MINOR 1)
set (Golden_VERSION_REV 0)

add_subdirectory(${PROJECT_SOURCE_DIR}/ext/glfw-3.0.3)
include_directories(${PROJECT_SOURCE_DIR}/ext/glfw-3.0.3/include)

subdirs(src)
