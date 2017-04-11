#version 140
///
// Particle fragment shader.
///
in vec4 v_color;

out vec4 o_color;

void main()
{
	o_color = v_color;
}