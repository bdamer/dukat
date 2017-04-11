#version 150
///
// Vertex shader to generate clipmap normal.
///
in vec4 a_position;
in vec2 a_tex_coord;

uniform int u_level;
uniform float u_size;
uniform float u_one_over_size;
// Torroidal texture offset of the current level.
uniform vec2 u_texture_offset;

out vec2 v_tex_coord;

void main()
{
    gl_Position = a_position;
    v_tex_coord = a_tex_coord * u_size;
}