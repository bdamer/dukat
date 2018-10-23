#version 330
precision mediump float;
///
// Mirror effect fragment shader.
///
in vec2 v_tex_coord;

// scene texture
uniform sampler2D u_tex0;
// mirror mask texture
uniform sampler2D u_tex1;

out vec4 o_color;

void main()
{
	vec4 original = texture(u_tex0, v_tex_coord);
	float reflectivity = texture(u_tex1, v_tex_coord).r;
    o_color = reflectivity * original;
}