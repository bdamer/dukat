#version 150
///
// Vertex shader to generate heatmap normal.
///
in vec4 a_position;
in vec2 a_tex_coord;

uniform float u_size;
uniform float u_one_over_size;

out vec2 v_tex_coord;

void main()
{
    gl_Position = a_position;
    v_tex_coord = a_tex_coord * u_size;
}
