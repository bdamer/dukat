#version 150
///
// Vertex shader for 2D text.
///
in vec2 a_position;
in vec4 a_color;
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

// out
out vec4 v_color;
out vec2 v_texCoord;

void main()
{
	v_color = a_color;
    v_texCoord = a_texCoord;
    gl_Position = u_cam.projOrth * u_model * vec4(a_position.x, -a_position.y, 0.0, 1.0);
}