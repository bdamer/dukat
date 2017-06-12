#version 150
///
// FBO example fragment shader.
///
in vec2 v_tex_coord;

uniform float u_time;

out vec4 o_color;

void main()
{
    o_color = vec4(sin(u_time * v_tex_coord.x), cos(u_time * v_tex_coord.y), 0, 1);
}