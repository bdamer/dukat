#version 150
///
// Skydome vertex shader.
///
in vec4 a_position;
in vec4 a_normal;

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

out vec3 v_tex_coord;

void main()
{
    v_tex_coord = a_position.xyz;
    // Move horizon down a little bit to provide better coverage and adjust for camera position
    vec4 pos = u_cam.proj_pers * u_cam.view * (u_model * (a_position - vec4(0, 0.2, 0, 0)) + u_cam.position);
    // Z=W to trick depth buffer into rendering skydome at far-z
    gl_Position = pos.xyww;
}