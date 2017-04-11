#version 140
///
// Clipmap debug fragment shader.
///
in vec3 v_tex_coord;

uniform vec4 u_color;
uniform sampler2DArray u_tex0;

out vec4 o_color;

void main()
{
	o_color = u_color;
}