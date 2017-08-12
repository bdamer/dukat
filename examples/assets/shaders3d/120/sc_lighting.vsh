#version 120
///
// Textured and lit vertex shader.
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

varying vec3 v_position;
varying vec3 v_normal;

void main()
{
	v_position = vec3(u_model * gl_Vertex);
	v_normal = normalize(gl_NormalMatrix * gl_Normal);
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position = u_cam_proj_pers * u_cam_view * vec4(v_position, 1.0);
}
