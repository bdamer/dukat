#version 150
///
// Skydome fragment shader.
///
in vec3 v_tex_coord;

// Mesh material is used for horizon and zenith colors.
layout(std140) uniform Material
{
	vec4 ambient;	// horizon
	vec4 diffuse;	// zenith
	vec4 specular;	// corona
	vec4 custom;	// [0] size of corona (larger values reduce size)
					// [1] blend-factor for u_tex0
} u_material;

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

uniform samplerCube u_tex0;

out vec4 o_color;

void main()
{
	vec3 light_pos = -normalize(u_lights.l[0].position.xyz);

	// Base is gradient between horizon and zenith color based on altitude
	vec4 color = mix(u_material.ambient, u_material.diffuse, v_tex_coord.y);

	// Sample texture and blend into color 
	vec4 t2 = texture(u_tex0, v_tex_coord);
	color = mix(color, t2, u_material.custom.g);

	// Include corona color based on light position
	float s = clamp(dot(v_tex_coord, light_pos), 0.0, 1.0);
	color = mix(color, u_material.specular, pow(s, u_material.custom.r));

	// TODO: dither color to avoid banding
	//	http://www.anisopteragames.com/how-to-fix-color-banding-with-dithering/
  	o_color = color;
}