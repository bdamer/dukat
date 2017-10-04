#version 150
///
// Fragment shader to generate normal map from heightmap.
///
in vec2 v_tex_coord;

// Elevation sampler
uniform sampler2D u_tex0;

// Size of normal texture
uniform float u_size;
// 1 / size of normal texture
uniform float u_one_over_size;
// -0.5 * (ratio of z to x,y grid spacing)
uniform vec2 u_grid_scale;

out vec4 o_color;

void main()
{
    // Need to floor uv coordinates to get pixel coordinate
    vec2 uv = floor(v_tex_coord);
	
    // compute a local tangent vector along the X axis
    vec2 texcoord0 = vec2(max(0, uv.x - 1), uv.y) * u_one_over_size;
    float z1 = texture(u_tex0, texcoord0 + 0.5 * u_one_over_size).r;  
    vec2 texcoord1 = vec2(min(u_size, uv.x + 1), uv.y) * u_one_over_size;
    float z2 = texture(u_tex0, texcoord1 + 0.5 * u_one_over_size).r;
    float zx = z2 - z1;
    
    // compute a local tangent vector along the Y axis
    texcoord0 = vec2(uv.x, max(0, uv.y - 1)) * u_one_over_size;
    z1 = texture(u_tex0, texcoord0 + 0.5 * u_one_over_size).r; 
    texcoord1 = vec2(uv.x, min(u_size, uv.y + 1)) * u_one_over_size;
    z2 = texture(u_tex0, texcoord1 + 0.5 * u_one_over_size).r;
    float zy = z2 - z1;
	
    // The normal is now the cross product of the two tangent vectors
    // normal = (2*sx, 0, zx) x (0, 2*sy, zy), where sx, sy = gridspacing in x, y
    // the normal below has n_z = 1
    // u_grid_scale = -0.5 * ScaleFac.z / ScaleFac.x, -0.5 * ScaleFac.z / ScaleFac.y
    vec2 normalf = vec2(zx * u_grid_scale.x, zy * u_grid_scale.y);
	
    // pack coordinates in [-1, +1] range to [0, 1] range
    normalf = normalf / 2.0 + 0.5;
   
    o_color = vec4(normalf, 0, 1);
}