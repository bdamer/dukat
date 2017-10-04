#version 150
///
// Textured, unlit fragment shader for heightmap.
///
// u,v texture coordinates, z-elevation
in vec3 v_tex_coord;

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

void main()
{
    // Set normal y-component to 1.0 to stay consistent with how the heatmap is rendered, 
    // i.e., y for elevation.
    vec2 nsample = texture(u_tex1, v_tex_coord.xy).xy;
    vec3 normal = vec3(nsample.x, 1.0, nsample.y);
    // unpack coordinates from [0, 1] to [-1. +1] range, and renormalize
    normal = normalize(normal * 2.0 - 1.0);

	// scalar determines texture repeat
	float tex_repeat = 2.0;

	// offsets into texture atlas
	float offset;
	float weight;
	
	// not great, but will do for now...
	if (v_tex_coord.z < 0.33)
	{
		offset = 0;
		weight = smoothstep(0.1, 0.33, v_tex_coord.z);
	}
	else if (v_tex_coord.z < 0.66)
	{
		offset = 1;
		weight = smoothstep(0.33, 0.66, v_tex_coord.z);
	}
	else
	{
		offset = 2;
		weight = smoothstep(0.66, 0.8, v_tex_coord.z);	
	}

	vec4 s1 = texture(u_tex2, vec3(tex_repeat * v_tex_coord.xy, offset));
	vec4 s2 = texture(u_tex2, vec3(tex_repeat * v_tex_coord.xy, offset + 1));
	vec4 color = mix(s1, s2, weight);

    o_color = vec4(dir_light(u_lights.l[0], normal, color), 1);
}