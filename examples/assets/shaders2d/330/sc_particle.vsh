#version 330
///
// Particle vertex shader.
///

// x,y,size,r
layout (location = 0) in vec4 a_position;
layout (location = 1) in vec4 a_color;

layout(std140) uniform Camera
{
	mat4 proj_orth;
	mat4 view;
    vec2 position;
	vec2 dimension;
} u_cam;

uniform float u_parallax;

// outputs
out vec4 v_color;

void main()
{
	// adjust view matrix for parallax:
	mat4 view = u_cam.view;
	view[3][0] *= u_parallax;
	view[3][1] *= u_parallax;	

	gl_Position = u_cam.proj_orth * view * vec4(a_position.xy, 0.0, 1.0); 
	gl_PointSize = a_position.z;	
	v_color = a_color;
}