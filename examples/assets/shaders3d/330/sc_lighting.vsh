#version 330
///
// Textured and lit vertex shader.
///
in vec4 a_position;
in vec4 a_normal;
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

out vec3 v_position;
out vec3 v_normal;
out vec2 v_tex_coord;

void main()
{
	v_position = vec3(u_model * a_position);
	v_normal = mat3(transpose(inverse(u_model))) * a_normal.xyz;
    v_tex_coord = a_tex_coord;
    gl_Position = u_cam.proj_pers * u_cam.view * vec4(v_position, 1.0);
}