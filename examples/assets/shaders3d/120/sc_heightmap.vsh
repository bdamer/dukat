#version 120
///
// Textured, unlit vertex shader for heightmap.
///

uniform mat4 u_cam_proj_pers;
uniform mat4 u_cam_proj_orth;
uniform mat4 u_cam_view;
uniform mat4 u_cam_view_inv;
uniform vec4 u_cam_position;
uniform vec4 u_cam_dir;
uniform vec4 u_cam_up;
uniform vec4 u_cam_left;

// Modelview matrix is used to pass in grid parameters:
// u_model[0].xy - Grid scale at current level [1..n]
// u_model[0].zw - Origin of current block (world space)
// u_model[1].xy - 1 / (w,h) of texture
// u_model[1].zw - undefined
// u_model[2].xy - undefined
// u_model[2].zw - undefined
// u_model[3].xy - undefined and ZScaleFactor
// u_model[3].zw - undefined
uniform mat4 u_model;

// Elevation sampler
uniform sampler2D u_tex0;
// Texture atlas
uniform sampler2D u_tex1;

void main()
{
	// Compute world position
    vec2 world_pos = gl_Vertex.xy * u_model[0].xy + u_model[0].zw;
    // sample elevation and store texture coordinates + elevation for fragment shader
    vec2 texcoord = gl_Vertex.xy * u_model[1].xy;
    float z = texture2D(u_tex0, texcoord).r;
	gl_TexCoord[0].stp = vec3(texcoord, z);
    // scale elevation and return vertex
    gl_Position = u_cam_proj_pers * u_cam_view * vec4(world_pos.x, z * u_model[3].y, world_pos.y, 1.0);
}