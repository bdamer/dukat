#version 120
///
// Skydome vertex shader.
// TODO: revisit - primitive restart is not working correctly in v120
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
	gl_TexCoord[0] = gl_Vertex.xyz;
    // Move horizon down a little bit to provide better coverage and adjust for camera position
    vec4 offset = vec4(0, -0.025, 0, 0);
    vec4 pos = u_cam_proj_pers * u_cam_view * (u_model * (gl_Vertex + offset) + u_cam_position);
    // Z=W to trick depth buffer into rendering skydome at far-z
    gl_Position = pos.xyww;
}