#version 150
///
// Textured vertex shader.
///
in vec4 a_position;
in vec4 a_normal;
in vec2 a_texCoord;

layout(std140) uniform Camera
{
	mat4 projPers;
	mat4 projOrth;
	mat4 view;
	mat4 viewInv;
	vec4 position;
	vec4 dir;
	vec4 up;
	vec4 left;
} u_cam;

uniform mat4 u_model;

out vec2 v_texCoord;

void main()
{
    v_texCoord = a_texCoord;
    gl_Position = u_cam.projPers * u_cam.view * u_model * a_position;
}