file(GLOB glew_src "*.c")
file(GLOB glew_inc "${GLEW_SOURCE_DIR}/include/GL/*.h")
add_library(glew ${glew_src} ${glew_inc})

add_definitions(-DGLEW_STATIC -DGLEW_MX)

set_target_properties(glew
    PROPERTIES
    LINKER_LANGUAGE CXX
)
