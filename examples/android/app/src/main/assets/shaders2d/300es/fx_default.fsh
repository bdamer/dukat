#version 300 es
precision mediump float;
///
// Passthrough effect fragment shader.
///
in vec2 v_tex_coord;

uniform sampler2D u_tex0;

out vec4 o_color;

void main()
{
    o_color = texture(u_tex0, v_tex_coord);
}