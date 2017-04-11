#version 120
///
// Vertex shader for 2D text.
///
uniform	mat4 u_cam_proj_orth;
uniform	mat4 u_cam_view;
uniform vec2 u_cam_position;
uniform	vec2 u_cam_dimension;

uniform mat4 u_model;

void main()
{
	gl_FrontColor = gl_Color;
    gl_Position = u_cam_proj_orth * u_model * gl_Vertex;
    gl_TexCoord[0] = gl_MultiTexCoord0;
}