#version 300 es
precision mediump float;
///
// Textured fragment shader.
///
in vec2 v_tex_coord;

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
	vec4 c = texture(u_tex0, v_tex_coord);
    o_color = u_material.ambient.rgba * c.rgba;
}