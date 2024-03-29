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

//#include "lib_water.fsh"

float caustic(vec2 uv, float speed)
{
	// Magic number
	const float tau = 6.28318530718;
	// Number of iterations
	const int max_iter = 5;
	// Scales size of caustics - larger numbers increase detail
	const float scale = 1.2;
	// Intensity value
	const float inten = .009;

	// Initialize values
	float time = speed * u_time + 23.0;
    vec2 p = mod(uv * tau, tau) - 250.0;
	vec2 i = vec2(p);
	float c = 1.0;

	for (int n = 0; n < max_iter; n++) 
	{
		float t = time * (1.0 - (3.5 / float(n + 1)));
		i = p + scale * vec2(cos(t - i.x) + sin(t + i.y), sin(t - i.y) + cos(t + i.x));
		c += 1.0 / length(vec2(p.x / (sin(i.x + t) / inten), p.y / (cos(i.y + t) / inten)));
	}

	c /= float(max_iter);
	c = 1.17 - pow(c, 1.4);
	c = pow(abs(c), 8.0);

    return clamp(c, 0.0, 1.0);
}

void main()
{
	// Compute caustic value
	float cval = caustic(v_tex_coord, 0.005);

	// Switch between one of three colors [0 - .5], [.5 - .75], [.75+]
	float s1 = step(0.75, cval);
	float s2 = step(0.5, cval);
	//o_color = (s1 * u_color_hi) + (1 - s1) * (s2 * u_color_mid + (1 - s2) * u_color_lo);
	o_color = mix(u_color_hi, mix(u_color_mid, u_color_lo, s2), s1);
	
}