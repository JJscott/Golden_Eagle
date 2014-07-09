/*
 *
 * Vertex and geometry shaders for rendering a single fullscreen tri
 *
 */

#version 330

#shader vertex
#shader geometry
#shader fragment

// vertex shader
#ifdef _VERTEX_

void main() { }

#endif

// geometry shader
#ifdef _GEOMETRY_

layout(points) in;
layout(triangle_strip, max_vertices = 3) out;

out vec2 texCoord;

void main() {
	// output a single triangle that covers the whole screen
	
	gl_Position = vec4(3.0, 1.0, 0.0, 1.0);
	texCoord = vec2(2.0, 1.0);
	EmitVertex();
	
	gl_Position = vec4(-1.0, 1.0, 0.0, 1.0);
	texCoord = vec2(0.0, 1.0);
	EmitVertex();
	
	gl_Position = vec4(-1.0, -3.0, 0.0, 1.0);
	texCoord = vec2(0.0, -1.0);
	EmitVertex();
	
	EndPrimitive();
	
}

#endif

// fragment shader
#ifdef _FRAGMENT_

in vec2 texCoord;

// main() should be implemented by includer

#endif
