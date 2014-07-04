
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

add_definitions(-DGLEW_STATIC -DGLEW_MX)