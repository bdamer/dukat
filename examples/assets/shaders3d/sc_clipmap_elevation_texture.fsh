#version 150
///
// Renders clipmap elevation sampler.
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
	// Render elevation sampler 
	vec4 c = texture(u_tex0, vec3(v_tex_coord, u_material.custom.r));
    o_color = vec4(c.r, c.r, c.r, 1);
}