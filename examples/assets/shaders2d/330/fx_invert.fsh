#version 330
precision mediump float;
///
// Fragment shader inverts colors.
///
in vec2 v_tex_coord;

uniform sampler2D u_tex0;

out vec4 o_color;

void main()
{
	vec4 c = texture(u_tex0, v_tex_coord);
    o_color = vec4(1 - c.rgb, c.a);
}