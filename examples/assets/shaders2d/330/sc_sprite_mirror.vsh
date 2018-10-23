#version 330
///
// Mirrored vertex shader for 2D sprites. Vertically flips texture 
// and shifts sprite down by 1 unit.
///

// X/Y and U/V base coordinates of this vertex.
layout (location = 0) in vec2 a_position;
layout (location = 1) in vec2 a_tex_coord;

layout(std140) uniform Camera
{
	mat4 proj_orth;
	mat4 view;
    vec2 position;
	vec2 dimension;
} u_cam;

uniform float u_parallax;
// Rect in texture map
uniform vec4 u_uvwh;
uniform mat4 u_model;

// outputs
out vec2 v_tex_coord;

void main()
{
    gl_Position = u_cam.proj_orth * u_cam.view * u_model * vec4(a_position.x, a_position.y + 1.0, 0.0, 1.0);
	v_tex_coord = u_uvwh.xy + u_uvwh.zw * vec2(a_tex_coord.x, 1.0 - a_tex_coord.y);
}
