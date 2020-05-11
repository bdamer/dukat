#version 150
///
// Passthrough effect fragment shader.
///
in vec2 v_tex_coord;

uniform sampler2D u_tex0;
uniform float u_phase;	// wave phase

out vec4 o_color;

// wave distortion
vec2 sine_wave(vec2 p)
{
	float x = sin(25.0 * p.y + 30.0 * p.x + 6.28 * u_phase) * 0.05;
	float y = sin(25.0 * p.y + 30.0 * p.x + 6.28 * u_phase) * 0.05;
	return vec2(p.x + x, p.y + y);
}

void main()
{
	o_color = texture(u_tex0, sine_wave(v_tex_coord));
}