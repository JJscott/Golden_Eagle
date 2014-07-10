/*
 *
 * Fragment shader base for rendering to scene buffer
 *
 */

#version 330

#shader fragment

// fragment shader
#ifdef _FRAGMENT_

uniform float zfar;

layout(location = 0) out float frag_z;
layout(location = 1) out vec4 frag_normal;
layout(location = 2) out vec4 frag_diffuse;
layout(location = 3) out vec4 frag_l0;

// call (one of) these functions to set fragment parameters
// note that depth_v > 0 for visible fragments!

void fragment_data(float depth_v, float tag, vec3 norm_v, vec3 diffuse, vec3 l0) {
	// this has to match with depth buffer settings from shadow shaders
	const float C = 0.01;
	float FC = 1.0 / log(zfar * C + 1.0);
	frag_z = depth_v;
	frag_normal = vec4(norm_v, tag);
	frag_diffuse.rgb = diffuse;
	frag_l0.rgb = max(l0, vec3(0.0));
	gl_FragDepth = log(depth_v * C + 1.0) * FC;
}

void fragment_data(float depth_v, vec3 norm_v, vec3 diffuse, vec3 l0) {
	fragment_data(depth_v, 0.0, norm_v, diffuse, l0);
}

void fragment_data(float depth_v, float tag, vec3 norm_v, vec3 diffuse) {
	fragment_data(depth_v, tag, norm_v, diffuse, vec3(0.0));
}

void fragment_data(float depth_v, vec3 norm_v, vec3 diffuse) {
	fragment_data(depth_v, norm_v, diffuse, vec3(0.0));
}

#endif
