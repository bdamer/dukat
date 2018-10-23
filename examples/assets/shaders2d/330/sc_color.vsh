#version 330
///
// Vertex shader for flat shaded geometry.
///
// x,y
layout (location = 0) in vec2 a_position;
layout (location = 1) in vec4 a_color;

layout(std140) uniform Camera
{
	mat4 proj_orth;
	mat4 view;
    vec2 position;
	vec2 dimension;
} u_cam;

uniform mat4 u_model;

out vec4 v_color;

void main()
{
	gl_Position = u_cam.proj_orth * u_cam.view * u_model * vec4(a_position.xy, 0.0, 1.0);
	v_color = a_color;
}