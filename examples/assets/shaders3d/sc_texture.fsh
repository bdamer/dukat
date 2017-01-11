#version 150
///
// Textured fragment shader.
///
in vec2 v_texCoord;

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
	vec4 c = texture(u_tex0, v_texCoord);
    o_color = u_material.ambient.rgba * c.rgba;
}