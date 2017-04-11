#version 150
///
// Renders clipmap normal texture.
///
in vec2 v_tex_coord;

layout(std140) uniform Material
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 custom;
} u_material;

uniform sampler2DArray u_tex0;

out vec4 o_color;

void main()
{
	vec4 c = texture(u_tex0, vec3(v_tex_coord, u_material.custom.r));
	o_color = vec4(c.r, c.g, 0, 1);
}