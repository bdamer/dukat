#version 330
///
// Vertex shader for UI elements. 
///
in vec3 a_position;
in vec2 a_tex_coord;

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

// out
out vec2 v_tex_coord;

void main()
{
    v_tex_coord = a_tex_coord;
    gl_Position = u_cam.proj_orth * u_model * vec4(a_position.x, a_position.y, 0.0, 1.0);
}