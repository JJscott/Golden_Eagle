file(GLOB glew_src "*.c")
add_library(glew ${glew_src})

add_definitions(-DGLEW_STATIC)

set_target_properties( glew
    PROPERTIES
    LINKER_LANGUAGE CXX
)