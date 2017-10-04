#version 150
///
// Textured, unlit vertex shader for heightmap.
///
in vec2 a_position;

layout(std140) uniform Camera
{
    mat4 proj_pers;
    mat4 proj_orth;
    mat4 view;
    mat4 view_inv;
    vec4 position;
    vec4 dir;
    vec4 up;
    vec4 left;
} u_cam;

// Modelview matrix is used to pass in grid parameters:
// u_model[0].xy - Grid scale at current level [1..n]
// u_model[0].zw - Origin of current block (world space)
// u_model[1].xy - 1 / (w,h) of texture
// u_model[1].zw - undefined
// u_model[2].xy - undefined
// u_model[2].zw - undefined
// u_model[3].xy - Base Z elevation and ZScaleFactor
// u_model[3].zw - Texture Scale and undefined
uniform mat4 u_model;

// Elevation sampler
uniform sampler2D u_tex0;
// Normal sampler
uniform sampler2D u_tex1;
// Texture atlas
uniform sampler2DArray u_tex2;

out vec3 v_tex_coord;
out vec4 v_world_pos;

void main()
{
	// Compute world position
    vec2 world_pos = a_position * u_model[0].xy + u_model[0].zw;
    // sample elevation and store texture coordinates + elevation for fragment shader
    vec2 texcoord = a_position * u_model[1].xy;
    float z = texture(u_tex0, texcoord).r;
  	v_tex_coord = vec3(texcoord, z);
    // scale elevation and return vertex
    v_world_pos = vec4(world_pos.x, u_model[3].x + z * u_model[3].y, world_pos.y, 1.0);
    gl_Position = u_cam.proj_pers * u_cam.view * v_world_pos;
}