#version 150
///
// Fullscreen fx pass vertex shader.
///
in vec4 a_position;
in vec2 a_tex_coord;

// Scale factor (0 for no scaling)
uniform float u_scale;

out vec2 v_tex_coord;

void main()
{
    gl_Position = a_position;
    v_tex_coord = a_tex_coord - gl_Position.xy * u_scale;
}