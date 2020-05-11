#version 150
///
// Color reduction effect shader.
///
in vec2 v_tex_coord;

uniform sampler2D u_tex0;

out vec4 o_color;

void main()
{
	vec4 c = texture(u_tex0, v_tex_coord);
	o_color = vec4(round(c.r * 8) / 8, round(c.g * 8) / 8, round(c.b * 8) / 8, c.a);
}