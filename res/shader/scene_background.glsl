/*
 *
 * Shader program for writing (black) background to scene buffer
 *
 */

#version 330

#shader fragment

#include "scene.glsl"
#include "fullscreen.glsl"

#ifdef _FRAGMENT_

void main() {
	fragment_data(zfar, vec3(0.0, 0.0, 1.0), vec3(0.0, 0.0, 0.0));
}

#endif
