#version 120
///
// Default vertex shader for 2D sprites.
///
uniform	mat4 u_cam_proj_orth;
uniform	mat4 u_cam_view;
uniform vec2 u_cam_position;
uniform	vec2 u_cam_dimension;

uniform float u_parallax;
// Rect in texture map
uniform vec4 u_uvwh;
uniform vec4 u_color;
uniform mat4 u_model;

void main()
{
	// adjust view matrix for parallax:
	mat4 view = u_cam_view;
	view[3][0] *= u_parallax;
	view[3][1] *= u_parallax;	
    gl_Position = u_cam_proj_orth * view * u_model * gl_Vertex;
	gl_TexCoord[0].st = u_uvwh.xy + u_uvwh.zw * gl_MultiTexCoord0.st;
}