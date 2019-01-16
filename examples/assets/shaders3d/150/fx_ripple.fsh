#version 150
///
// Ripple effect fragment shader.
// Samples from current and last ripple map to distribute
// and dampen ripples across out texture.
///
in vec2 v_tex_coord;

// current ripple map
uniform sampler2D u_tex0;
// last ripple map
uniform sampler2D u_tex1;

// Size of texture
uniform float u_size;
// 1 / u_size
uniform float u_one_over_size;
// dampening factor
uniform float u_dampening;

out vec4 o_color;

void main()
{
	// Last elevation at this location
	float cur = texture(u_tex0, v_tex_coord).r;
	// Sample surrounding elevations
	float last = texture(u_tex1, v_tex_coord - vec2(0, u_one_over_size)).r + 
		texture(u_tex1, v_tex_coord + vec2(u_one_over_size, 0)).r + 
		texture(u_tex1, v_tex_coord + vec2(0, u_one_over_size)).r + 
		texture(u_tex1, v_tex_coord - vec2(u_one_over_size, 0)).r;
	float z = 0.5 * last - cur;
	
	// dampen
	z -= u_dampening * z;
	
	o_color = vec4(z, 0, 0, 1);
}