#version 150
///
// Vertex shader to update clipmap elevation sampler.
///
in vec4 a_position;
in vec2 a_tex_coord;

// Texture size
uniform vec2 u_size;
uniform vec2 u_one_over_size;
// Rect in elevation sampler to update.
uniform vec4 u_rect;

out vec2 v_tex_coord;

void main()
{
    v_tex_coord = a_position.xy * u_rect.zw * u_one_over_size;
    vec2 pos = vec2(-1.0, -1.0) + 2.0 * u_rect.xy * u_one_over_size + 2.0 * a_position.xy * u_rect.zw * u_one_over_size;
    gl_Position = vec4(pos, 0, 1);
}