#version 150
///
// FBO example fragment shader.
///
in vec2 v_tex_coord;

uniform float u_time;

out vec4 o_color;

#define ITERATIONS 300
#define HALF_ITERATIONS 150

// Real & imaginary constant
uniform vec2 u_k;
uniform vec2 u_offset;
uniform float u_zoom;

void main()
{
	// Determine real and imaginary component
	float r = (v_tex_coord.x - 0.5) / u_zoom - u_offset.x;
	float i = (v_tex_coord.y - 0.5) / u_zoom - u_offset.y;
	
	int j;
	for (j = 0; j < ITERATIONS; j++)
	{
		float r2 = r * r;
		float i2 = i * i;
		
		if (r2 + i2 > 4)
			break;			
		
		i = 2 * r * i + u_k.y;
		r = r2 - i2 + u_k.x;
	}

	if (j < HALF_ITERATIONS)
	{
		o_color = vec4(j * (1.0 / HALF_ITERATIONS), 0, 0, 1.0);
	}
	else
	{
		o_color = vec4(1.0, 
			(i - HALF_ITERATIONS) * (1 / HALF_ITERATIONS), 
			(i - HALF_ITERATIONS) * (1 / HALF_ITERATIONS), 
			1.0);
	}
}