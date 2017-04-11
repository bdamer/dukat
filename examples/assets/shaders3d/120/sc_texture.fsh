#version 120
///
// Textured fragment shader.
///
uniform	vec4 u_material_ambient;
uniform	vec4 u_material_diffuse;
uniform	vec4 u_material_specular;
uniform	vec4 u_material_custom;

uniform sampler2D u_tex0;

void main()
{
	vec4 c = texture2D(u_tex0, gl_TexCoord[0].st);
    gl_FragColor = u_material_ambient.rgba * c.rgba;
}