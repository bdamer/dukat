#version 330
precision mediump float;
///
// Textured, unlit fragment shader for heightmap.
///
// u,v texture coordinates, z-elevation
in vec3 v_tex_coord;
in vec4 v_world_pos;

struct Light {
	vec4 position;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
    vec4 params;
};

layout(std140) uniform Lights {
	Light l[5];
} u_lights;

uniform mat4 u_model;

// Elevation sampler
uniform sampler2D u_tex0;
// Normal sampler
uniform sampler2D u_tex1;
// Texture atlas
uniform sampler2DArray u_tex2;

out vec4 o_color;

// Computes simplified ambient and diffuse lighting.
vec3 dir_light(Light light, vec3 normal, vec4 color)
{
    vec3 light_dir = normalize(-light.position.xyz);
    // diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);
    // combine results
    vec3 ambient = light.ambient.rgb * color.rgb;
    vec3 diffuse = light.diffuse.rgb * diff * color.rgb;
	return (ambient + diffuse);
}

#include "lib_triplanar.fsh"

void main()
{
    // Set normal y-component to 1.0 to stay consistent with how the heatmap is rendered, 
    // i.e., y for elevation.
    vec2 nsample = texture(u_tex1, v_tex_coord.xy).xy;
    vec3 normal = vec3(nsample.x, 1.0, nsample.y);
    // unpack coordinates from [0, 1] to [-1. +1] range, and renormalize
    normal = normalize(normal * 2.0 - 1.0);

	// offsets into texture atlas
	float offset;
	float weight;
	
	// not great, but will do for now...
	if (v_tex_coord.z < 0.25)
	{
		offset = 1;
		weight = smoothstep(0.0, 0.25, v_tex_coord.z);
	}
	else
	{
		offset = 2;
		weight = smoothstep(0.25, 1.0, v_tex_coord.z);	
	}

	vec3 blending = compute_blending(normal);

	vec4 xaxis1 = texture(u_tex2, vec3(u_model[3].z * v_world_pos.yz, offset));
	vec4 yaxis1 = texture(u_tex2, vec3(u_model[3].z * v_world_pos.xz, offset));
	vec4 zaxis1 = texture(u_tex2, vec3(u_model[3].z * v_world_pos.xy, offset));
	vec4 s1 = xaxis1 * blending.x + yaxis1 * blending.y + zaxis1 * blending.z;

	vec4 xaxis2 = texture(u_tex2, vec3(u_model[3].z * v_world_pos.yz, offset + 1));
	vec4 yaxis2 = texture(u_tex2, vec3(u_model[3].z * v_world_pos.xz, offset + 1));
	vec4 zaxis2 = texture(u_tex2, vec3(u_model[3].z * v_world_pos.xy, offset + 1));
	vec4 s2 = xaxis2 * blending.x + yaxis2 * blending.y + zaxis2 * blending.z;

	vec4 color = mix(s1, s2, weight);

    o_color = vec4(dir_light(u_lights.l[0], normal, color), 1);
}