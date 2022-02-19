#version 120
///
// Fragment shadow for shadows.
///
uniform float u_radius;
uniform float u_alpha;
uniform vec2 u_size;

void main()
{
	// Adjust height to account for pinching done in vertex shader
	vec2 size = vec2(u_size.x, u_size.y * 0.5);
	// Quantize to pixel size
	vec2 uv = round(gl_TexCoord[0].st * size) / size;
	// Offset uv with the center of the circle.
	uv = uv - vec2(0.5, 0.5);

	float dist = sqrt(dot(uv, uv));
	if (dist < u_radius)
		gl_FragColor = vec4(0, 0, 0, u_alpha);
	else
		gl_FragColor = vec4(0, 0, 0, 0);
}