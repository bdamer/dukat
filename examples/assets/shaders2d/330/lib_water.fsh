
// Adapted from https://www.shadertoy.com/view/MdlXz8# by David Hoskins.
// Based on Water turbulence effect by joltz0r 2013-07-04, improved 2013-07-07

// Returns caustic ripple value.
// uv Texture coordinate [0..1]
// time variable
float caustic(vec2 uv, float time)
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