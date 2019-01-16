#version 150
///
// Textured ripple fragment shader. 
// Applies refraction and lighting to background texture.
///
in vec2 v_tex_coord;

layout(std140) uniform Material
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	vec4 custom;
} u_material;

// Background texture
uniform sampler2D u_tex0;
// Ripple texture
uniform sampler2D u_tex1;

out vec4 o_color;

void main()
{
	// max ripple amplitude is stored in u_material.custom.r
	float max_amplitude = u_material.custom.r;
	// size and one over size 
	float u_one_over_size = u_material.custom.b;
	float u_ref_index = u_material.custom.a;	

	// Sample elevation
	float z = texture(u_tex1, v_tex_coord).r;
	
	// Compute cotangent
	float zx = texture(u_tex1, v_tex_coord + vec2(u_one_over_size, 0)).r;
	float zy = texture(u_tex1, v_tex_coord + vec2(0, u_one_over_size)).r;
	float xd = (zx - z) / max_amplitude;
	float xangle = atan(xd);
	float xref = asin(sin(xangle) / u_ref_index);
	float xdispl = tan(xref);
	float yd = (zy - z) / max_amplitude;
	float yangle = atan(yd);
	float yref = asin(sin(yangle) / u_ref_index  );
	float ydispl = tan(yref);

	// Sample texture at offset based on refraction
	vec2 offset = vec2(sign(xd) * xdispl, sign(yd) * ydispl);
	vec4 c = texture(u_tex0, v_tex_coord + offset);

	// scale z value into [0.5..1.0] range
	z = (z + max_amplitude) / (max_amplitude);
	
	// apply color modification to sampled fragment
    o_color = vec4(z * c.rgb, c.a);
}