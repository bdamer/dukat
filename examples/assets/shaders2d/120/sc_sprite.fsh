#version 120
///
// Default frament shader for 2D sprites.
// Samples a texture and multiplies in a uniform color.
///
uniform vec4 u_color;
uniform sampler2D u_tex0;

void main()
{
	vec4 material = texture2D(u_tex0, gl_TexCoord[0].st);
	gl_FragColor = u_color * material;
}