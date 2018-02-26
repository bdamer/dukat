#version 140
///
// Flat shaded fragment shader.
///
in vec4 v_color;
in vec4 v_world_pos;

out vec4 o_color;

#include "lib_noise2d.fsh"

void main()
{
	vec4 color_plain = vec4(0.8941, 0.7216, 0.4431, 1.0);
	vec4 color_hill = vec4(0.6078, 0.4627, 0.3255, 1.0);
	vec4 color_mountain = vec4(0.5137, 0.4941, 0.4863, 1.0);
	vec4 color_snow = vec4(1.0, 1.0, 1.0, 1.0);
	
	float z = v_world_pos.y;

	float weight;
	if (z < 0.5)
	{
		o_color = color_plain;
		o_color.g += 0.5 * v_color.b;
		o_color.rb -= 0.5 * v_color.b;
	}
	else if (z < 0.6)
	{
		weight = smoothstep(0.5, 0.6, z);
		o_color = mix(color_plain, color_hill, weight);
		o_color.g += 0.5 * v_color.b;
		o_color.rb -= 0.5 * v_color.b;
	}
	else if (z < 0.7)
	{
		weight = smoothstep(0.6, 0.7, z);
		o_color = mix(color_hill, color_mountain, weight);
		o_color.g += 0.5 * v_color.b;
		o_color.rb -= 0.5 * v_color.b;
	}
	else if (z < 0.8)
	{
		weight = smoothstep(0.7, 0.8, z);
		o_color = mix(color_mountain, color_snow, weight);
	}
	else
	{
		o_color = color_snow;
	}
	
	// Add some noise to make color look a little more interesting
	float n = random(v_world_pos.xz);
	o_color.rgb += n * 0.10 - 0.05;
}