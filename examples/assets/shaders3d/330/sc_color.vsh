#version 330
///
// Flat shaded, unlit vertex shader.
///
in vec4 a_position;
in vec4 a_color;

layout(std140) uniform Camera
{
    mat4 proj_pers;
    mat4 proj_orth;
    mat4 view;
    mat4 view_inv;
    vec4 position;
    vec4 dir;
    vec4 up;
    vec4 left;
} u_cam;

uniform mat4 u_model;

out vec4 v_color;

void main()
{
    v_color = a_color;
    gl_Position = u_cam.proj_pers * u_cam.view * u_model * a_position;
}