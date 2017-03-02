#version 150
///
// Fragment shader to update clipmap elevation sampler.
///
in vec2 v_tex_coord;

// Update sampler
uniform sampler2D u_tex0;

out vec4 o_color;

void main()
{
    o_color = texture(u_tex0, v_tex_coord);
}