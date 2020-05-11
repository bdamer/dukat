#version 330
precision mediump float;
///
// Color reduction effect shader.
///
in vec2 v_tex_coord;

uniform sampler2D u_tex0;
uniform float u_phase;	// wave phase

out vec4 o_color;

void main()
{
	vec4 c = texture(u_tex0, v_tex_coord);
	float cr = sin(u_phase);
	float cg = cos(u_phase);
	float cb = tan(u_phase);
	o_color = vec4(cr * c.r, cg * c.g, cb * c.b, c.a);
}