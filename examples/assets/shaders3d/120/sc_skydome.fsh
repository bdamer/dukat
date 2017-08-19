#version 120
///
// Skydome fragment shader.
// Note: simplified version of v150.
///

// Mesh material is used for horizon and zenith colors.
uniform	vec4 u_material_ambient;
uniform	vec4 u_material_diffuse;
uniform	vec4 u_material_specular;
uniform	vec4 u_material_custom;

uniform sampler2D u_tex0;

void main()
{
	// Base is gradient between horizon and zenith color based on altitude
	vec4 color = mix(u_material_ambient, u_material_diffuse, gl_TexCoord[0].t);

	// Sample texture and blend into color 
	vec4 t2 = texture2D(u_tex0, gl_TexCoord[0].st);
	color = mix(color, t2, u_material_custom.g);

    gl_FragColor = color;	
}