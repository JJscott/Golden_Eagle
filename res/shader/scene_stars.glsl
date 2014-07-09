/*
 *
 * Shader program for writing stars to scene buffer
 *
 */

#version 330

#shader vertex
#shader fragment

#include "scene.glsl"

#ifdef _VERTEX_

uniform mat4 univViewMatrix;
uniform mat4 projectionMatrix;

// xyz: direction, w: radiance (average radiance as seen at a fragment)
layout(location = 0) in vec4 star_data;

out VertexData {
	float l;
} vertex_out;

void main() {
	gl_Position = projectionMatrix * univViewMatrix * vec4(star_data.xyz, 0.0);
	vertex_out.l = star_data.w;
}

#endif

#ifdef _FRAGMENT_

in VertexData {
	float l;
} vertex_in;

void main() {
	fragment_data(
		zfar,
		vec3(0.0, 0.0, 1.0),
		vec3(0.0, 0.0, 0.0),
		vec3(vertex_in.l)
	);
}

#endif
