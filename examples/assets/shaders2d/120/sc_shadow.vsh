#version 120
///
// Vertex shader for shadows.
///
uniform	mat4 u_cam_proj_orth;
uniform	mat4 u_cam_view;
uniform vec2 u_cam_position;
uniform	vec2 u_cam_dimension;

uniform mat4 u_model;

void main()
{
	// Shift down and pinch on the y-axis
	gl_Position = u_cam_proj_orth * u_cam_view * u_model * vec4(gl_Vertex.x, 0.5 * gl_Vertex.y + 0.5, 0.0, 1.0);
	gl_TexCoord[0].st = gl_MultiTexCoord0.st;
}