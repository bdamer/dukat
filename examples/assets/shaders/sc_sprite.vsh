#version 140
///
// Default vertex shader for 2D sprites.
///

// X/Y and U/V base coordinates of this vertex.
in vec2 a_position;
in vec2 a_texCoord;

layout(std140) uniform Camera
{
	mat4 proj_orth;
	mat4 view;
    vec2 position;
	vec2 dimension;
} u_cam;

uniform float u_parallax;
// Rect in texture map
uniform vec4 u_uvwh;
uniform vec4 u_color;
uniform mat4 u_model;

// outputs
out vec2 v_texCoord;

void main()
{
	// adjust view matrix for parallax:
	mat4 view = u_cam.view;
	view[3][0] *= u_parallax;
	view[3][1] *= u_parallax;	
    gl_Position = u_cam.proj_orth * view * u_model * vec4(a_position, 0.0, 1.0);
	v_texCoord = u_uvwh.xy + u_uvwh.zw * a_texCoord;
}