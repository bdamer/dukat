#version 120
///
// Textured fragment shader for text rendering.
///
uniform sampler2D u_tex0;

void main()
{
	vec4 c = texture2D(u_tex0, gl_TexCoord[0].st);
    gl_FragColor = vec4(gl_Color.rgb, gl_FrontMaterial.ambient.a) * c.rgba;
}