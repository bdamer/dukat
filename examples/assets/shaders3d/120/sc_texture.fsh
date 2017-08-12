#version 120
///
// Textured fragment shader.
///
uniform sampler2D u_tex0;

void main()
{
	vec4 c = texture2D(u_tex0, gl_TexCoord[0].st);
    gl_FragColor = gl_FrontMaterial.ambient * c.rgba;
}