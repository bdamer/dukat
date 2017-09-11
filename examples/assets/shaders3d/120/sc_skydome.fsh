#version 120
///
// Skydome fragment shader.
// Note: simplified version of v150.
///

uniform sampler2D u_tex0;

void main()
{
	// Mesh material is used for horizon and zenith colors.

	// Base is gradient between horizon and zenith color based on altitude
	vec4 color = mix(gl_FrontMaterial.ambient, gl_FrontMaterial.diffuse, gl_TexCoord[0].t);

	// Sample texture and blend into color 
	vec4 t2 = texture2D(u_tex0, gl_TexCoord[0].st);
	color = mix(color, t2, gl_FrontMaterial.emission.g);

    gl_FragColor = color;	
}