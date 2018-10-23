#version 330
/**
 * Vertex shader for debug rendering.
 */
layout (location = 0) in vec2 a_position;

layout(std140) uniform Camera
{
	mat4 proj_orth;
	mat4 view;
    vec2 position;
	vec2 dimension;
} u_cam;

uniform mat4 u_model;

void main()
{
	//gl_Position = u_cam.proj_orth * u_cam.view * u_model * vec4(a_position.xy, 0.0, 1.0);
	gl_Position = u_cam.proj_orth * u_cam.view * vec4(a_position.xy, 0.0, 1.0);
}