
#version 330 core

#include "test2.glsl"

#shader vertex
#shader fragment

#ifdef _VERTEX_

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;

uniform mat4 MVP;

out vec2 UV;

void main() {
	gl_Position = MVP * vec4(vertexPosition_modelspace, 1);
	UV = vertexUV;
}

#endif

#ifdef _FRAGMENT_

in vec2 UV;
out vec3 color;

uniform sampler2D myTextureSampler;

void main() {
	color = texture(myTextureSampler, UV).rgb;
	//color = vec3(1, 0, 0);
}

#endif

