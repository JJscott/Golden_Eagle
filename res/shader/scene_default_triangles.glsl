/*
 *
 * Default shader program for writing to scene buffer using GL_TRIANGLES
 *
 */

#version 330

#shader vertex
#shader geometry
#shader fragment

#include "scene.glsl"

#ifdef _VERTEX_

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

// input: model-space position
layout(location = 0) in vec3 pos_m;

// output: view-space position
out VertexData {
	vec3 pos_v;
} vertex_out;

void main() {
	vec3 pos_v = (modelViewMatrix * vec4(pos_m, 1.0)).xyz;
	vertex_out.pos_v = pos_v;
	gl_Position = projectionMatrix * vec4(pos_v, 1.0);
}

#endif

#ifdef _GEOMETRY_

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VertexData {
	vec3 pos_v;
} vertex_in[];

out VertexData {
	vec3 pos_v;
	vec3 norm_v;
} vertex_out;

void main() {
	
	// compute face normal from vertices
	vertex_out.norm_v = normalize(cross(vertex_in[1].pos_v - vertex_in[0].pos_v, vertex_in[2].pos_v - vertex_in[1].pos_v));
	
	// emit triangle
	gl_Position = gl_in[0].gl_Position;
	vertex_out.pos_v = vertex_in[0].pos_v;
	EmitVertex();
	gl_Position = gl_in[1].gl_Position;
	vertex_out.pos_v = vertex_in[1].pos_v;
	EmitVertex();
	gl_Position = gl_in[2].gl_Position;
	vertex_out.pos_v = vertex_in[2].pos_v;
	EmitVertex();
	EndPrimitive();
	
}

#endif

#ifdef _FRAGMENT_

in VertexData {
	vec3 pos_v;
	vec3 norm_v;
} vertex_in;

void main() {
	fragment_data(
		-vertex_in.pos_v.z,
		normalize(vertex_in.norm_v),
		vec3(1.0, 0.0, 0.0)
	);
}

#endif
