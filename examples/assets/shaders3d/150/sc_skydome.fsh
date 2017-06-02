#version 150
///
// Skydome fragment shader.
///
in vec3 v_tex_coord;

// Mesh material is used for horizon and zenith colors.
layout(std140) uniform Material
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
	vec3 custom;
} u_material;

uniform sampler2D u_tex0;

out vec4 o_color;

void main()
{
    o_color = mix(u_material.ambient, u_material.diffuse, v_tex_coord.y);
}