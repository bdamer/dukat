#version 150
///
// Vertex shader for shadows.
///

// X/Y and U/V base coordinates of this vertex.
in vec2 a_position;
in vec2 a_tex_coord;

layout(std140) uniform Camera
{
	mat4 proj_orth;
	mat4 view;
    vec2 position;
	vec2 dimension;
} u_cam;

uniform mat4 u_model;

out vec2 v_tex_coord;

void main()
{
	// Shift down and pinch on the y-axis
    gl_Position = u_cam.proj_orth * u_cam.view * u_model * vec4(a_position.x, 0.5 * a_position.y + 0.5, 0.0, 1.0);
	v_tex_coord = a_tex_coord;
}