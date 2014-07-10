/*
 *
 * Default shader program for writing to scene buffer using GL_TRIANGLES_ADJACENCY
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
layout(location = 1) in vec3 norm_m;
layout(location = 2) in vec3 tang_m;
layout(location = 3) in vec3 tex_pos_m;

// output: view-space position
out VertexData {
	vec3 pos_v;
	vec3 norm_v;
	vec3 tang_v;
	vec3 tex_pos_v;
} vertex_out;

void main() {
	vec3 pos_v = (modelViewMatrix * vec4(pos_m, 1.0)).xyz;
	vec3 norm_v = (modelViewMatrix * vec4(norm_m, 0.0)).xyz;
	vec3 tang_v = (modelViewMatrix * vec4(tang_m, 0.0)).xyz;
	vertex_out.pos_v = pos_v;
	vertex_out.norm_v = norm_v;
	vertex_out.tang_v = tang_v;
	vertex_out.tex_pos_v = tex_pos_m;
	gl_Position = projectionMatrix * vec4(pos_v, 1.0);
}

#endif

#ifdef _GEOMETRY_

layout(triangles_adjacency) in;
layout(triangle_strip, max_vertices = 3) out;

in VertexData {
	vec3 pos_v;
	vec3 norm_v;
	vec3 tang_v;
	vec3 tex_pos_v;
} vertex_in[];

out VertexData {
	vec3 pos_v;
	vec3 norm_v;
	vec3 tang_v;
	vec3 tex_pos_v;
} vertex_out;

void main() {

	// emit triangle
	gl_Position = gl_in[0].gl_Position;
	vertex_out.pos_v = vertex_in[0].pos_v;
	vertex_out.norm_v = vertex_in[0].norm_v;
	vertex_out.tang_v = vertex_in[0].tang_v;
	vertex_out.tex_pos_v = vertex_in[0].tex_pos_v;
	EmitVertex();

	gl_Position = gl_in[2].gl_Position;
	vertex_out.pos_v = vertex_in[2].pos_v;
	vertex_out.norm_v = vertex_in[2].norm_v;
	vertex_out.tang_v = vertex_in[2].tang_v;
	vertex_out.tex_pos_v = vertex_in[2].tex_pos_v;
	EmitVertex();

	gl_Position = gl_in[4].gl_Position;
	vertex_out.pos_v = vertex_in[4].pos_v;
	vertex_out.norm_v = vertex_in[4].norm_v;
	vertex_out.tang_v = vertex_in[4].tang_v;
	vertex_out.tex_pos_v = vertex_in[4].tex_pos_v;
	EmitVertex();

	EndPrimitive();
}

#endif

#ifdef _FRAGMENT_

uniform mat4 viewPlanetMatrix;
uniform sampler2D sampler_diffuse;
uniform sampler2D sampler_normal;

in VertexData {
	vec3 pos_v;
	vec3 norm_v;
	vec3 tang_v;
	vec3 tex_pos_v;
} vertex_in;

void main() {
	// gl_Position = ftransform();
	// vec4 worldPos = matWorld * gl_Vertex;//So we obtain the world position

	// TexCoordX = worldPos.zy/tileSize;//here are our texture coordinates...
	// TexCoordY = worldPos.xz/tileSize;
	// TexCoordZ = worldPos.xy/tileSize;

	// //binormal and tangent become normal dependant

	// vec3 xtan,ytan,ztan;
	// vec3 xbin,ybin,zbin;

	// xtan = vec3(0,0,1);//tangent space for the X aligned plane
	// xbin = vec3(0,1,0);

	// ytan = vec3(1,0,0);//tangent space for the Y aligned plane
	// ybin = vec3(0,0,1);

	// ztan = vec3(1,0,0);//tangent space for the Z aligned plane
	// zbin = vec3(0,1,0);

	// vec3 n = gl_Normal;
	// n*=n;

	// vec3 worldNormal = gl_Normal;
	// vec3 worldBinormal = xbin*n.x+ybin*n.y+zbin*n.z;//Average Binormal
	// vec3 worldTangent = xtan*n.x+ytan*n.y+ztan*n.z;//Average Tangent

	// ViewDir = (worldPos-eyePosition).xyz;

	// //This is done so it can be rotated freely

	// worldTangent = (matWorld*vec4(worldTangent,1)).xyz;
	// worldBinormal = (matWorld*vec4(worldBinormal,1)).xyz;
	// worldNormal = (matWorld*vec4(worldNormal,1)).xyz;

	// tangentSpace[0] = worldTangent;
	// tangentSpace[1] = worldBinormal;
	// tangentSpace[2] = worldNormal;


	// vec3 n = normal;
	// n*=n;

	vec3 norm = normalize(vertex_in.norm_v);
	vec4 n2 = normalize(viewPlanetMatrix * vec4(norm, 0));
	n2 *= n2;


	vec4 col = texture(sampler_diffuse, vertex_in.tex_pos_v.zy) * n2.x +
				texture(sampler_diffuse, vertex_in.tex_pos_v.xz) * n2.y +
				texture(sampler_diffuse, vertex_in.tex_pos_v.xy) * n2.z;


	vec3 tang = normalize(vertex_in.tang_v);
	vec3 bitang = cross(tang, norm);

	vec3 bumpNormal = (texture(sampler_normal, vertex_in.tex_pos_v.zy) * n2.x +
				texture(sampler_normal, vertex_in.tex_pos_v.xz) * n2.y +
				texture(sampler_normal, vertex_in.tex_pos_v.xy) * n2.z).xyz;

	//range from [0,1] -> [-1,1]
	bumpNormal = 2.0 * bumpNormal - vec3(1.0, 1.0, 1.0);

	mat3 tbn = mat3(tang, bitang, norm);
	vec3 newnorm = normalize(tbn * bumpNormal);






	// vec4 nrm = (2.0*texture2D(normalX,TexCoordX)-1.0)*n.x+
	// 	(2.0*texture2D(normalY,TexCoordY)-1.0)*n.y+
	// 	(2.0*texture2D(normalZ,TexCoordZ)-1.0)*n.z;

	// vec3 realNormal = normalize(tangentSpace*nrm.xyz);
	// vec3 lightDir = normalize(lightDirection);
	// vec3 viewDir = normalize(ViewDir);

	// float NdL = max(0.0,dot(realNormal,lightDir));

	// vec3 reflVect = normalize(reflect(lightDir,realNormal));
	// float RdL = max(0.0,dot(reflVect,viewDir));

	// vec4 ambient,diffuse,specular,fresnel;

	// ambient = col*ambientColor;
	// diffuse = diffuseColor*col*NdL;
	// specular = specularColor*pow(RdL,specularPower)*col;
	// fresnel = max(0.0,dot(realNormal,viewDir))*specularColor;

	// gl_FragColor = ambient+diffuse+specular+fresnel;



	fragment_data(
		-vertex_in.pos_v.z, //depth
		newnorm, //normal
		col.xyz * 0.2
		// vec3(0.075, 0.25, 0.025) //diffuse
	);
}

#endif
