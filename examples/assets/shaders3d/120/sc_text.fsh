#version 120
///
// Textured fragment shader for text rendering.
///
uniform	vec4 u_material_ambient;
uniform	vec4 u_material_diffuse;
uniform	vec4 u_material_specular;
uniform	vec4 u_material_custom;

uniform sampler2D u_tex0;

void main()
{
	vec4 c = texture2D(u_tex0, gl_TexCoord[0].st);
    gl_FragColor = vec4(gl_Color.rgb, u_material_ambient.a) * c.rgba;
}