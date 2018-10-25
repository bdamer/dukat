#version 300 es
precision mediump float;
///
// Default frament shader for 2D sprites.
// Samples a texture and multiplies in a uniform color.
///
in vec2 v_tex_coord;

uniform vec4 u_color;
uniform sampler2D u_tex0;

out vec4 o_color;

void main()
{
	vec4 material = texture(u_tex0, v_tex_coord);
	o_color = u_color * material;
}