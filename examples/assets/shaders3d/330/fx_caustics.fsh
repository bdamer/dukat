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
	float cval = caustic(v_tex_coord, 0.005);

	// Switch between one of three colors [0 - .5], [.5 - .75], [.75+]
	float s1 = step(0.75, cval);
	float s2 = step(0.5, cval);
	o_color = (s1 * u_color_hi) + (1 - s1) * (s2 * u_color_mid + (1 - s2) * u_color_lo);
}