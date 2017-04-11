#version 120
///
// Textured vertex shader.
///
uniform mat4 u_cam_proj_pers;
uniform mat4 u_cam_proj_orth;
uniform mat4 u_cam_view;
uniform mat4 u_cam_view_inv;
uniform vec4 u_cam_position;
uniform vec4 u_cam_dir;
uniform vec4 u_cam_up;
uniform vec4 u_cam_left;

uniform mat4 u_model;

void main()
{
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position = u_cam_proj_pers * u_cam_view * u_model * gl_Vertex;
}