#version 300 es
precision mediump float;
///
// Ground effect used by layers example.
///
in vec2 v_tex_coord;

layout(std140) uniform Camera
{
	mat4 proj_orth;
	mat4 view;
    vec2 position;
	vec2 dimension;
} u_cam;

// scene texture
uniform sampler2D u_tex0;
// ground texture
uniform sampler2D u_tex1;

// aspect ratio
uniform float u_aspect;
// scale factor
uniform float u_scale;

out vec4 o_color;

void main()
{	
	o_color = texture(u_tex1, v_tex_coord * vec2(u_scale, u_scale / u_aspect));
}