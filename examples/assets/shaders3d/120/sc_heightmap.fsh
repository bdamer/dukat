#version 120
///
// Textured, unlit fragment shader for heightmap.
///

// Elevation sampler
uniform sampler2D u_tex0;
// Texture atlas
uniform sampler2D u_tex1;

void main()
{
	// scalar determines texture repeat
	float tex_repeat = 0.5;

	// elevation determines which textures to use
	float height = gl_TexCoord[0].p;
	
	// offsets into texture atlas
	vec2 offset1;
	vec2 offset2;
	float weight;
	
	// not great, but will do for now...
	if (height < 0.33)
	{
		offset1 = vec2(0,0);
		offset2 = vec2(0.5, 0);
		weight = smoothstep(0.1, 0.33, height);
	}
	else if (height < 0.66)
	{
		offset1 = vec2(0.5,0);
		offset2 = vec2(0, 0.5);
		weight = smoothstep(0.33, 0.66, height);
	}
	else
	{
		offset1 = vec2(0,0.5);
		offset2 = vec2(0.5, 0.5);
		weight = smoothstep(0.66, 0.8, height);	
	}

	vec4 s1 = texture2D(u_tex1, offset1 + tex_repeat * gl_TexCoord[0].st);
	vec4 s2 = texture2D(u_tex1, offset2 + tex_repeat * gl_TexCoord[0].st);
	gl_FragColor = mix(s1, s2, weight);
}