
# get source files
# we could list these manually...
file(GLOB ambition_src "*.cpp" "*.c")
file(GLOB ambition_hdr "*.hpp" "*.h")

add_library(ambition ${ambition_src} ${ambition_hdr})

set_target_properties(
	ambition
    PROPERTIES
    LINKER_LANGUAGE CXX
)

# list libs that must be linked with ambition
list(APPEND AMBITION_LIBRARIES_impl glew glfw ${GLFW_LIBRARIES})
if(WIN32)
	list(APPEND AMBITION_LIBRARIES_impl wsock32 Ws2_32)
endif()

# list macros that must be defined when using ambition
list(APPEND AMBITION_DEFINITIONS_impl -DGLEW_STATIC -DGLEW_MX)

# export
set(AMBITION_LIBRARIES ${AMBITION_LIBRARIES_impl} CACHE STRING "Ambition required libraries" FORCE)
set(AMBITION_DEFINITIONS ${AMBITION_DEFINITIONS_impl} CACHE STRING "Ambition required definitions" FORCE)

add_definitions(${AMBITION_DEFINITIONS})