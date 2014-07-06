
# allow targets to include from each other
include_directories("${GOLDEN_SOURCE_DIR}/src")

# this needs to be done first
add_subdirectory("./ambition")

add_subdirectory("./test")
add_subdirectory("./game")
add_subdirectory("./server")

