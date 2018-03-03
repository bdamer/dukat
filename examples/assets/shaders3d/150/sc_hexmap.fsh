#version 140
///
// Flat shaded fragment shader.
///
in vec4 v_color;
in vec2 v_world_pos;

out vec4 o_color;

#include "lib_noise2d.fsh"

void main()
{
	vec4 color_plain = vec4(0.8941, 0.7216, 0.4431, 1.0);
	vec4 color_hill = vec4(0.6078, 0.4627, 0.3255, 1.0);
	vec4 color_mountain = vec4(0.5137, 0.4941, 0.4863, 1.0);
	vec4 color_snow = vec4(1.0, 1.0, 1.0, 1.0);
	vec4 color_vegetation = vec4(0.043, 0.40, 0.137, 1.0);
	vec4 color_water = vec4(0.251, 0.643, 0.875, 1.0);

	float moisture = v_color.b;
	float elevation = v_color.r;
	float river = step(0.99, moisture); // 1 - river, 0 - land

	float weight;
	vec4 color_ground;
	if (elevation < 0.5)
	{
		color_ground = color_plain;
	}
	else if (elevation < 0.6)
	{
		weight = smoothstep(0.5, 0.6, elevation);
		color_ground = mix(color_plain, color_hill, weight);
	}
	else if (elevation < 0.7)
	{
		weight = smoothstep(0.6, 0.7, elevation);
		color_ground = mix(color_hill, color_mountain, weight);
	}
	else if (elevation < 0.8)
	{
		weight = smoothstep(0.7, 0.8, elevation);
		color_ground = mix(color_mountain, color_snow, weight);
	}
	else
	{
		color_ground = color_snow;
	}

	// Add vegetation based on moisture & elevation
	float scaled_elev = min(1.0, (elevation / 1.1));
	color_ground = mix(color_ground, color_vegetation, (1 - scaled_elev) * moisture);

	// Add some noise to make color look a little more interesting
	float n = random(v_world_pos);
	color_ground.rgb += n * 0.05 - 0.025;

	// Final color is either water or ground color	
	o_color = mix(color_ground, color_water, river);
}