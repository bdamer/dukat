#version 150
///
// Wave effect vertex shader.
///
in vec4 a_position;
in vec2 a_tex_coord;

uniform vec4 u_rescale; // used to scale back to 0..1
uniform mat4 u_rtex_coord; // ripple texture coords
uniform mat4 u_coef; // wave coefficients
uniform float u_pass; // index of current pass

// Consine lookup table
uniform sampler1D u_tex0;
// Noise map
uniform sampler2D u_tex1;
// Normal map
uniform sampler2D u_tex2;

// Texture coordinates
out vec2 v_tex_coord;
// 4 per-wave coordinates to lookup cos values
out vec4 v_cos_coord;

void main()
{
    gl_Position = a_position;
    v_tex_coord = a_tex_coord;
    v_cos_coord.x = dot(vec4(a_tex_coord, 1, 1), u_rtex_coord[0]);
    v_cos_coord.y = dot(vec4(a_tex_coord, 1, 1), u_rtex_coord[1]);
    v_cos_coord.z = dot(vec4(a_tex_coord, 1, 1), u_rtex_coord[2]);
    v_cos_coord.w = dot(vec4(a_tex_coord, 1, 1), u_rtex_coord[3]);
}