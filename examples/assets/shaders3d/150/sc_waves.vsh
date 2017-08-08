#version 150
///
// Geometry wave vertex shader. 
// Based on Chapter 1 of GPU Gems.
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

struct Wave
{
	float phase;
	float amp;
	float len;
	float freq;
	float fade;
	float dirx;
	float diry;
	float dirz;
};

layout(std140) uniform Waves
{
	Wave w[4];
} u_waves;

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

struct WaveState
{
	vec4 water_tint;
	// Vector of water_level+1,water_level+1,water_level,water_level
	vec4 depth_offset;
	vec4 fog_params;
	vec4 spec_atten;
	vec4 env_adjust;
};
uniform WaveState u_ws;

uniform float u_k;

// u,v texture coordinates in bump map
out vec2 v_tex_coord;
// Color mod of reflected env map
out vec4 v_mod_color;
// Base color (tint)
out vec4 v_add_color;
// binormal, tangent, normal matrix
out mat3 v_btn;
// eye-to-vertex in tangent space
out vec3 v_eye_ray;

// Calculates sines and cosines for our 4 waves and returns them to the caller.
void calc_sin_cos(const in vec4 pos, const in Wave wave, const in float inv_edge_length,
	const in float scale, out float sine, out float cosine)
{
	const float pi = 3.14159265f;
	const float two_pi = 2.0f * pi;
	const float inv_two_pi = 1.0f / two_pi;
	
	// Dot x and y with direction vectors
	float dist = wave.dirx * pos.x + wave.diry * pos.y;
	// Scale in our frequency and add in our phase
	dist = dist * wave.freq + wave.phase;
	// Mod into range [-Pi..Pi]
	dist = fract((dist + pi) * inv_two_pi) * two_pi - pi;
	// Compute powers
	float dist2 = dist * dist;
	float dist3 = dist2 * dist;
	float dist4 = dist2 * dist2;
	float dist5 = dist3 * dist2;
	float dist6 = dist3 * dist3;
	float dist7 = dist4 * dist3;

	sine = dist - dist3 / 6.0 + dist5 / 120.0 - dist7 / 5040.0;
	cosine = 1.0 - dist2 / 2.0 + dist4 / 24.0 - dist6 / 720.0;

	float filtered_amp = clamp(wave.len * inv_edge_length, 0.0f, 1.0f) * scale * wave.amp;

	sine = sine * filtered_amp;
	cosine = cosine * filtered_amp * scale;
}

vec4 calc_final_position(in vec4 pos, const in vec4 sines, const in vec4 cosines,
	const in float depth_offset, const in vec4 dir_xk, const in vec4 dir_yk)
{
	// Sum to a scalar
	float h = dot(sines, vec4(1,1,1,1)) + depth_offset;

	pos.x += dot(cosines, dir_xk);
	pos.y += dot(cosines, dir_yk);
	// Clamp to never go beneath input height
	pos.z = max(pos.z, h);

	// Swap y & z to adhere to OpenGL convention that y is up
	float t = pos.y;
	pos.y = pos.z;
	pos.z = t;

	return pos;
}

void calc_eye_ray_and_bump_attenuation(const in vec4 pos, const in vec4 cam_pos, const in vec4 spec_atten,
	out vec3 cam_to_vertex, out float pert_atten)
{
	// Get normalized vec from camera to vertex, saving original distance.
	cam_to_vertex = pos.xyz - cam_pos.xyz;
	pert_atten = length(cam_to_vertex);
	cam_to_vertex = cam_to_vertex / pert_atten;

	// Calculate our normal perturbation attenuation. This attenuation will be
	// applied to the horizontal components of the normal read from the computed
	// ripple bump map, mostly to fight aliasing. This doesn't attenuate the 
	// color computed from the normal map, it attenuates the "bumps".
	pert_atten = pert_atten + spec_atten.x;
	pert_atten = pert_atten * spec_atten.y;
	pert_atten = clamp(pert_atten, 0.0, 1.0);
	pert_atten = pert_atten * pert_atten; // Square it to account for perspective.
	pert_atten = pert_atten * spec_atten.z;
}

// Compute our finitized eyeray.
vec3 finitize_eye_ray(const in vec3 cam_to_vtx, const in vec4 env_adjust)
{
	// Our "finitized" eyeray is:
	//	camPos + D * t - envCenter = D * t - (envCenter - camPos)
	// with
	//	D = (pos - camPos) / |pos - camPos| // normalized usual eyeray
	// and
	//	t = D dot F + sqrt( (D dot F)^2 - G )
	// with
	//	F = (envCenter - camPos)	=> envAdjust.xyz
	//	G = F^2 - R^2				=> nevAdjust.w
	// where R is the sphere radius.
	//
	// This all derives from the positive root of equation
	//	(camPos + (pos - camPos) * t - envCenter)^2 = R^2,
	// In other words, where on a sphere of radius R centered about envCenter
	// does the ray from the real camera position through this point hit.
	//
	// Note that F and G are both constants (one 3-point, one scalar).
	float dot_f = dot(cam_to_vtx, env_adjust.xyz);
	float t = dot_f + sqrt(dot_f * dot_f - env_adjust.w);
	return cam_to_vtx * t - env_adjust.xyz;
}

// Computes BTN matrix
// btn[0] - binormal
// btn[1] - tangent
// btn[2] - normal
void calc_tangent_basis(const in vec4 sines, const in vec4 cosines,
	const in vec4 dirx_sqkw, const in vec4 dirx_diry_kw, const in vec4 diry_sqkw,
	const in vec4 dirx_w, const in vec4 diry_w, const in vec4 kw, const in float pert_atten, out mat3 btn)
{
	/*
	Okay, here we go:
	W == sum(k w Dir.x^2 A sin())
	V == sum(k w Dir.x Dir.y A sin())
	U == sum(k w Dir.y^2 A sin())

	T == sum(A sin())

	S == sum(k Dir.x A cos())
	R == sum(k Dir.y A cos())

	Q == sum(k w A cos())

	M == sum(A cos())

	P == sum(w Dir.x A cos())
	N == sum(w Dir.y A cos())

	Then:
	Pos = (in.x + S, in.y + R, waterheight + T)

	Bin = (1 - W, -V, P)
	Tan = (-V, 1 - U, N)
	Nor = (-P, -N, 1 - Q)

	Remember we want the transpose of Binormal, Tangent, and Normal
	*/
	
	btn = mat3(
		(1.0 + dot(sines, -dirx_sqkw)) * pert_atten, -dot(cosines, dirx_w) * pert_atten, dot(sines, -dirx_diry_kw) * pert_atten,
		dot(sines, -dirx_diry_kw) * pert_atten, -dot(cosines, diry_w) * pert_atten, (1.0 + dot(sines, -diry_sqkw)) * pert_atten,
		dot(cosines, -dirx_w), -(1.0 + dot(sines, -kw)), dot(cosines, -diry_w)
	);
}

void calc_final_colors(const in vec3 norm, const in vec3 cam_to_vertex, const in float opac_min, const in float opac_scale,
	const in float color_filter, const in float opac_filter, const in vec4 tint, out vec4 mod_color, out vec4 add_color)
{
	// Calculate colors
	// Final color will be
	// rgb = Color1.rgb + Color0.rgb * envMap.rgb
	// alpha = Color0.a

	// Color 0

	// Vertex based Fresnel-esque effect.
	// Input vertex color.b limits how much we attenuate based on angle.
	// So we map 
	// (dot(norm,cam2Vtx)==0) => 1 for grazing angle
	// and (dot(norm,cam2Vtx)==1 => 1-In.Color.b for perpendicular view.
	float atten = 1.0 + dot(norm, cam_to_vertex) * opac_min;

	// Filter the color based on depth
	mod_color.rgb = vec3(color_filter * atten);

	// Boost the alpha so the reflections fade out faster than the tint
	// and apply the input attenuation factors.
	mod_color.a = (atten + 1.0) * 0.5 * opac_filter * opac_scale * tint.a;

	// Color 1 is just a constant.
	add_color = tint;
}

void main()
{
	// Input vertex format should be:
	// xyz where z is elevation of ground 
	// depth of water is given by water_table - z

	// vertex color controls: [default: 1/1/1/1]
	// r - make surface transparent as it goes to 0
	// g - modulates reflection strength / matte as g goes to 0
	// b - opacity attenuation
	// a - distance between vertices? seems like that is captured as a uniform
	vec4 a_color = vec4(1, 1, 1, 1);

	// Compute world position
	vec4 world_pos = vec4(a_position * u_model[0].xy + u_model[0].zw, 0.0, 1.0);

	// Calculate ripple UV from position
	v_tex_coord = vec2(world_pos.xy * u_ws.spec_atten.ww);

	// Calculate depth based filters.
	// depth_filter.x => overall opacity
	// depth_filter.y => reflection strength
	// depth_filter.z => wave height
	// scale of 0.5 is from original "depthScale" but was moved into shader completely
	vec3 depth_filter = clamp(0.5 * (u_ws.depth_offset.xyz - world_pos.zzz), 0.0, 1.0);

	// Build our 4 waves
	vec4 sines, cosines;
	calc_sin_cos(world_pos, u_waves.w[0], a_color.a, depth_filter.z, sines.x, cosines.x);
	calc_sin_cos(world_pos, u_waves.w[1], a_color.a, depth_filter.z, sines.y, cosines.y);
	calc_sin_cos(world_pos, u_waves.w[2], a_color.a, depth_filter.z, sines.z, cosines.z);
	calc_sin_cos(world_pos, u_waves.w[3], a_color.a, depth_filter.z, sines.w, cosines.w);

	// Compute direction vectors
	vec4 w = vec4(u_waves.w[0].freq, u_waves.w[1].freq, u_waves.w[2].freq, u_waves.w[3].freq);
	vec4 dirx = vec4(u_waves.w[0].dirx, u_waves.w[1].dirx, u_waves.w[2].dirx, u_waves.w[3].dirx);
	vec4 diry = vec4(u_waves.w[0].diry, u_waves.w[1].diry, u_waves.w[2].diry, u_waves.w[3].diry);
	vec4 kw = u_k * w;
	vec4 dir_xk = u_k * dirx;
	vec4 dir_yk = u_k * diry;
	vec4 dir_xw = dirx * w;
	vec4 dir_yw = diry * w;
	vec4 dir_xsqkw = dirx * dirx * kw;
	vec4 dir_ysqkw = diry * diry * kw;
	vec4 dir_xykw = dirx * diry * kw;

	world_pos = calc_final_position(world_pos, sines, cosines, u_ws.depth_offset.w, dir_xk, dir_yk);
	
	// We have our final position. We'll be needing normalized vector from camera 
	// to vertex several times, so we go ahead and grab it.
	vec3 cam_to_vertex;
	float pert_atten;
	calc_eye_ray_and_bump_attenuation(world_pos, u_cam.position, u_ws.spec_atten, cam_to_vertex, pert_atten);

	// Compute our finitized eyeray.
	v_eye_ray = finitize_eye_ray(cam_to_vertex, u_ws.env_adjust);

	calc_tangent_basis(sines, cosines, dir_xsqkw, dir_xykw, dir_ysqkw, dir_xw, dir_yw, kw, pert_atten, v_btn);

	// Calc screen position and fog from screen W
	mat4 mat_pv = u_cam.proj_pers * u_cam.view;
	gl_Position = mat_pv * world_pos;
	// Fog is basic linear from start distance to end distance.
	// float fog = (gl_Position.w + u_ws.fog_params.x) * u_ws.fog_params.y;

	calc_final_colors(v_btn[2], cam_to_vertex, a_color.b, a_color.r,
		depth_filter.y, depth_filter.x, u_ws.water_tint, v_mod_color, v_add_color);
}
