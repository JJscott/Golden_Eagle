/*
 *
 * Albireo: main deferred shader program
 *
 */

#version 330

#shader vertex
#shader fragment

#include "fullscreen.glsl"

#define ATMOS_PI 3.141592654

// fragment shader
#ifdef _FRAGMENT_

uniform uint show_overdraw;

uniform float noise_time;
uniform float dither;

// inverse projection transform
uniform mat4 inv_proj_matrix;

// far plane
uniform float zfar;

// scene buffer
uniform sampler2D sampler_z;
uniform sampler2D sampler_normal;
uniform sampler2D sampler_diffuse;
uniform sampler2D sampler_l0;
uniform usampler2D sampler_stencil;

// light (sun) and planet positions
uniform vec3 light_norm_v;
uniform vec3 planet_pos_v;

// light source radiance (outside atmosphere) and solid angle
uniform vec3 light_l;
const float light_sa = 6.87e-5;

// exposure for HDR
uniform float exposure;

out vec4 frag_color;

vec3 hdr(vec3 l) {
	return 1.0 - exp(-exposure * l);
}

void main() {
	vec4 temp;
	
	// read scene info
	float depth_v = texture(sampler_z, texCoord).r;
	
	temp = texture(sampler_normal, texCoord);
	vec3 norm_v = temp.xyz;
	float tag = temp.w;
	temp = texture(sampler_diffuse, texCoord);
	vec3 diffuse = temp.rgb;

	// reconstruct scene pos from depth
	temp = inv_proj_matrix * vec4(texCoord * 2.0 - 1.0, 0.0, 1.0);
	vec3 pos_v = temp.xyz * (-depth_v / temp.z);
	float dist_v = length(pos_v);

	// interval to eval atmos inscatter over
	vec3 dp = normalize(pos_v);
	
	// direct radiance
	vec3 ld = light_l * 1.0 * 1.0;
	// direct irradiance
	float light_dot = dot(light_norm_v, norm_v);
	vec3 ed = ld * light_sa * max(0.0, light_dot);
	// indirect irradiance
	vec3 ei = light_l * light_sa * 0.0;

	vec3 l0 = vec3(0.0);

	// reflected diffuse radiance
	l0 += (ed + ei) * diffuse / (2.0 * ATMOS_PI);

	// emissivity
	l0 += texture(sampler_l0, texCoord).rgb;

	// add inscatter and reflection
	vec3 l = l0;
	
	// direct light from the sun
	if (depth_v >= zfar && dot(light_norm_v, dp) > 1.0 - light_sa / (2.0 * ATMOS_PI)) {
		// this seems far too bright... but the units are correct now, i think
		l += light_l * 1.0;
	}

	// holy shit this function is brilliant
	// http://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
	float junk = fract(sin(dot(gl_FragCoord.xy + noise_time, vec2(12.9898, 78.233))) * 43758.5453);

	frag_color = vec4(hdr(l) + vec3((junk - 0.5) / dither), 1.0);

	if (texture(sampler_stencil, texCoord).r >= show_overdraw) {
		frag_color = vec4(1.0 - frag_color.rgb, 1.0);
	}

}

#endif

