#version 120
///
// Particle vertex shader.
///
uniform	mat4 u_cam_proj_orth;
uniform	mat4 u_cam_view;
uniform vec2 u_cam_position;
uniform	vec2 u_cam_dimension;

uniform float u_parallax;

void main()
{
	// adjust view matrix for parallax:
	mat4 view = u_cam_view;
	view[3][0] *= u_parallax;
	view[3][1] *= u_parallax;	

	gl_Position = u_cam_proj_orth * view * vec4(gl_Vertex.xy, 0.0, 1.0); 
	gl_PointSize = gl_Vertex.z;	
	gl_FrontColor = gl_Color;
}