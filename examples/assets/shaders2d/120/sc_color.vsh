#version 120
///
// Vertex shader for flat shaded geometry.
///
uniform	mat4 u_cam_proj_orth;
uniform	mat4 u_cam_view;
uniform vec2 u_cam_position;
uniform	vec2 u_cam_dimension;

uniform mat4 u_model;

void main()
{
	gl_Position = u_cam_proj_orth * u_cam_view * u_model * vec4(gl_Vertex.xy, 0.0, 1.0); 
	gl_FrontColor = gl_Color;
}