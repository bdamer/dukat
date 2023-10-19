#version 330
precision mediump float;
///
// Water caustics effect fragment shader.
///
in vec2 v_tex_coord;

// time
uniform float u_time;
uniform vec4 u_color_lo;
uniform vec4 u_color_mid;
uniform vec4 u_color_hi;

out vec4 o_color;

#include "lib_water.fsh"

void main()
{
	// Compute caustic value
	float cval = caustic(v_tex_coord, u_time);
	o_color = vec4(cval, 0, 0, 1);
}