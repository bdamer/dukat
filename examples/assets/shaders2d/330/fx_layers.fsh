#version 330
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
// mask texture 
uniform sampler2D u_tex2;
// caustics texture 
uniform sampler2D u_tex3;

// Colors
uniform vec4 u_color_lo;
uniform vec4 u_color_mid;
uniform vec4 u_color_hi;

// aspect ratio
uniform float u_aspect;
// scale factor
uniform float u_scale;

out vec4 o_color;

void main()
{	
	vec4 base_color = texture(u_tex1, v_tex_coord * vec2(u_scale, u_scale / u_aspect));

	// convert screen position into cam space 
	// denormalize [ from 0..1 to -0.5..0.5 ]
	vec2 screen_offset = vec2(v_tex_coord.x, 1 - v_tex_coord.y) - 0.5;
	// stretch mask texture to cover most of our window
	vec2 uv = (screen_offset * vec2(1.0 / u_aspect, 1.0)) + 0.5;

	float blend = texture(u_tex2, uv).r;
	float cval = texture(u_tex3, uv).r;
	
	// Switch between one of three colors [0 - .5], [.5 - .75], [.75+]
	float s1 = step(0.75, cval);
	float s2 = step(0.5, cval);
	vec4 caustic_color = (s1 * u_color_hi) + (1 - s1) * (s2 * u_color_mid + (1 - s2) * u_color_lo);

	o_color = vec4(mix(base_color.xyz, caustic_color.xyz, blend), 1);
}