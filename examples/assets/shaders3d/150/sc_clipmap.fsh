#version 150
///
// Clipmap shaded fragment shader.
///
// u,v texture coordinates, z-elevation
in vec3 v_tex_coord;
// alpha (blend)
in float v_alpha;

layout(std140) uniform Light {
	vec4 position;
	vec4 color;
    vec4 params;
} u_light;

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

// Elevation sampler
uniform sampler2DArray u_tex0;
// Normal sampler
uniform sampler2DArray u_tex1;

// Color sampler
uniform sampler1D u_tex2;

// Debug color - not used
uniform vec4 u_color;

// Debug flags
// x - Blending
// y - Lighting
uniform vec2 u_debug;

// Texture offset of current level (xy) and next coarser level (zw)
uniform vec4 u_texture_offset;
// Shift of current level within next coarser block in texture space
uniform vec2 u_offset;

out vec4 o_color;

void main()
{
    // 1st light is directional
    vec3 light_dir = -u_light.position.xyz;
    
    // sample normal texture at current, fine level
    vec2 normal_f = texture(u_tex1, vec3(v_tex_coord.xy , u_model[3].z)).xy;
	// sample normal texture at the next coarser level 
	vec2 texcoordc = fract(v_tex_coord.xy + 1.0) / 2.0 + u_offset;
	vec2 normal_c = texture(u_tex1, vec3(texcoordc, u_model[3].z + 1)).xy;
    // blend normals using alpha computed in vertex shader
    vec2 normal_mixed = mix(normal_f.xy, normal_c.xy, v_alpha);
	
    // Set normal y-component to 1.0 to stay conssiten with how the clipmap is rendered, 
    // i.e., y for elevation.
    vec3 normal = vec3(normal_mixed.x, 1.0, normal_mixed.y);
    
    // unpack coordinates from [0, 1] to [-1. +1] range, and renormalize
    normal = normalize(normal * 2.0 - 1.0);

    // compute dot product of light and normal
    float s = clamp(dot(normal, light_dir), 0.0, 1.0);

	if (u_debug.y == 0.0)
	{
		s = 1.0;
	}
	
    // assign terrain color based on its elevation
    vec4 color = texture(u_tex2, v_tex_coord.z);
	o_color = vec4(s * color.rgb, color.a);
}