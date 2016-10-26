#version 140
///
// Vertex shader for 2D text.
///
in vec2 a_position;
in vec4 a_color;
in vec2 a_texCoord;

layout(std140) uniform Camera
{
	mat4 proj_orth;
	mat4 view;
    vec2 position;
	vec2 dimension;
} u_cam;

uniform mat4 u_model;

// out
out vec4 v_color;
out vec2 v_texCoord;

void main()
{
	v_color = a_color;
    v_texCoord = a_texCoord;
    gl_Position = u_cam.proj_orth * u_model * vec4(a_position, 0.0, 1.0);
}