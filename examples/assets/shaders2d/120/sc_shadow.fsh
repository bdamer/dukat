#version 120
///
// Fragment shadow for shadows.
///
uniform float u_radius;
uniform float u_alpha;

void main()
{
	// Offset uv with the center of the circle.
	vec2 uv = gl_TexCoord[0].st - vec2(0.5, 0.5);
	  
	float dist = sqrt(dot(uv, uv));
	if (dist < u_radius)
		gl_FragColor = vec4(0, 0, 0, u_alpha);
	else
		gl_FragColor = vec4(0, 0, 0, 0);
}