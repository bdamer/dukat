#version 140
///
// Fullscreen glow composite fx pass.
///
in vec2 v_tex_coord;
// Scene texture
uniform sampler2D u_tex0;
// Effects texture
uniform sampler2D u_tex1;
out vec4 o_color;

void main()
{
    vec4 scene = texture(u_tex0, v_tex_coord);  
    vec4 fx = texture(u_tex1, v_tex_coord);
    o_color = 1.0 - (1.0 - scene) * (1.0 - fx);
}