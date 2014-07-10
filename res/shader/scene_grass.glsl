/*
 *
 * Shader program for writing grass to scene buffer
 *
 */

#version 330

#shader vertex
#shader geometry
#shader fragment

#include "scene.glsl"

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

uniform sampler2D sampler_grass;

#ifdef _VERTEX_

// possible inputs
// - size
// - texture atlas index (flip horizontally?)
// - normal (get from terrain?)
// - colour
// - wind

// xyz: position, w: size
layout(location = 0) in vec4 pos_m;

out VertexData {
	vec3 pos_m;
	float size;
} vertex_out;

void main() {
	vertex_out.pos_m = pos_m.xyz;
	vertex_out.size = pos_m.w;
}

#endif

#ifdef _GEOMETRY_

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in VertexData {
	vec3 pos_m;
	float size;
} vertex_in[];

out VertexData {
	vec3 pos_v;
	vec3 norm_v;
	vec2 uv;
} vertex_out;

void main() {
	
	float size = vertex_in[0].size;
	if (size < 0.1) return;

	vec3 pos_v = (modelViewMatrix * vec4(vertex_in[0].pos_m, 1.0)).xyz;
	vec3 up_v = normalize((modelViewMatrix * vec4(0.0, 1.0, 0.0, 0.0)).xyz);
	vec3 side_v = normalize(cross(normalize(pos_v), up_v));

	vertex_out.norm_v = up_v;

	vertex_out.pos_v = pos_v - 0.5 * size * side_v;
	vertex_out.uv = vec2(0.0, 0.0);
	gl_Position = projectionMatrix * vec4(vertex_out.pos_v, 1.0);
	EmitVertex();
	vertex_out.pos_v = pos_v + 0.5 * size * side_v;
	vertex_out.uv = vec2(1.0, 0.0);
	gl_Position = projectionMatrix * vec4(vertex_out.pos_v, 1.0);
	EmitVertex();
	vertex_out.pos_v = pos_v - 0.5 * size * side_v + size * up_v;
	vertex_out.uv = vec2(0.0, 1.0);
	gl_Position = projectionMatrix * vec4(vertex_out.pos_v, 1.0);
	EmitVertex();
	vertex_out.pos_v = pos_v + 0.5 * size * side_v + size * up_v;
	vertex_out.uv = vec2(1.0, 1.0);
	gl_Position = projectionMatrix * vec4(vertex_out.pos_v, 1.0);
	EmitVertex();
	EndPrimitive();

}

#endif

#ifdef _FRAGMENT_

in VertexData {
	vec3 pos_v;
	vec3 norm_v;
	vec2 uv;
} vertex_in;

void main() {
	vec4 diffuse = texture(sampler_grass, vertex_in.uv);
	if (diffuse.a < 0.5) discard;

	if (vertex_in.pos_v.z < -50) discard;

	fragment_data(
		-vertex_in.pos_v.z,
		normalize(vertex_in.norm_v),
		diffuse.rgb * 0.2
	);
}

#endif