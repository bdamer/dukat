#version 150
///
// Textured, unlit fragment shader for heightmap.
///
// u,v texture coordinates, z-elevation
in vec3 v_tex_coord;

uniform mat4 u_model;

// Elevation sampler
uniform sampler2D u_tex0;
// Texture atlas
uniform sampler2DArray u_tex1;

out vec4 o_color;

void main()
{
	// scalar determines texture repeat
	float tex_repeat = 2.0;

	// offsets into texture atlas
	float offset;
	float weight;
	
	// not great, but will do for now...
	if (v_tex_coord.z < 0.33)
	{
		offset = 0;
		weight = smoothstep(0.1, 0.33, v_tex_coord.z);
	}
	else if (v_tex_coord.z < 0.66)
	{
		offset = 1;
		weight = smoothstep(0.33, 0.66, v_tex_coord.z);
	}
	else
	{
		offset = 2;
		weight = smoothstep(0.66, 0.8, v_tex_coord.z);	
	}

	vec4 s1 = texture(u_tex1, vec3(tex_repeat * v_tex_coord.xy, offset));
	vec4 s2 = texture(u_tex1, vec3(tex_repeat * v_tex_coord.xy, offset + 1));
	o_color = mix(s1, s2, weight);
}