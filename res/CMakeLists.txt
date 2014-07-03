# add resources as targets so they show up in IDEs

file(GLOB golden_shader "${GOLDEN_SOURCE_DIR}/res/shader/*")

add_custom_target(
	res
	SOURCES
	${golden_shader}
)

source_group("shader" FILES ${golden_shader})
