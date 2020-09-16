#version 150
///
// Fragment shadow for shadows.
///
in vec2 v_tex_coord;

uniform vec4 u_color;
uniform float u_radius;
uniform float u_alpha;

out vec4 o_color;

void main()
{
	// Offset uv with the center of the circle.
	vec2 uv = v_tex_coord - vec2(0.5, 0.5);
	  
	float dist = sqrt(dot(uv, uv));
	if (dist < u_radius)
		o_color = vec4(0, 0, 0, u_color.a * u_alpha);
	else
		o_color = vec4(0, 0, 0, 0);
}