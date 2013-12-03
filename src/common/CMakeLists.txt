file(GLOB common_src "*.cpp")
file(GLOB common_header "*.hpp")
add_library(common ${common_src} ${common_header})

set_target_properties( common
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LINKER_LANGUAGE CXX
)
