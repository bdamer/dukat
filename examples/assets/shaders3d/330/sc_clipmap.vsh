#version 330
///
// Clipmap vertex shader.
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
// u_model[1].zw - Origin of current block (texture space)
// u_model[2].xy - Observer position (world space)
// u_model[2].zw - Alpha offset 
// u_model[3].xy - 1/w (blend) and ZScaleFactor
// u_model[3].zw - index of current level and max level
uniform mat4 u_model;

// Elevation samplers
uniform sampler2DArray u_tex0;
// Normal sampler
uniform sampler2DArray u_tex1;
// Color sampler
uniform sampler1D u_tex2;

// Debug flags
// x - Blending
// y - Lighting
uniform vec2 u_debug;

// Torroidal offset of current level (xy) and next coarser level (zw) in texture space
uniform vec4 u_texture_offset;
// Shift of current level within next coarser block in texture space
uniform vec2 u_offset;

out vec3 v_tex_coord;
out float v_alpha;

void main()
{
    // Compute world position
    vec2 world_pos = a_position * u_model[0].xy + u_model[0].zw;

    // Compute vertex texture coordinates
    vec2 texcoordf = a_position * u_model[1].xy + u_model[1].zw;

	// sample the vertex texture
	float zf = texture(u_tex0, vec3(texcoordf + u_texture_offset.xy, u_model[3].z)).r;
	
	// sample the vertex texture at the next coarser level
	vec2 texcoordc = fract(texcoordf + 1.0) / 2.0 + u_texture_offset.zw + u_offset;
	float zc = texture(u_tex0, vec3(texcoordc, u_model[3].z + 1)).r;
	
	// compute alpha (transition parameter), and blend elevation.
    vec2 alpha = clamp((abs(world_pos - u_model[2].xy) - u_model[2].zw) * u_model[3].x, 0, 1);
    // u_debug.x allows us to disable blending, also make sure to disable if level == max_level
    v_alpha = max(alpha.x, alpha.y) * u_debug.x * clamp(u_model[3].w - u_model[3].z, 0, 1);
    
	// blend fine and coarse levels
	float z = mix(zf, zc, v_alpha);

	v_tex_coord = vec3(texcoordf, z);

    gl_Position = u_cam.proj_pers * u_cam.view * vec4(world_pos.x, z * u_model[3].y, world_pos.y, 1.0);
}