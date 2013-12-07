file(GLOB server_src "*.cpp" "*.c")
file(GLOB server_header "*.hpp" "*.h")
add_library(server ${server_src} ${server_header})

set_target_properties( server
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LINKER_LANGUAGE CXX
)
