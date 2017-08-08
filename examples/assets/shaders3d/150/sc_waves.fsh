#version 150
///
// Geometry wave fragment shader.
///

// u,v texture coordinates in bump map
in vec2 v_tex_coord;
// Color mod of reflected env map
in vec4 v_mod_color;
// Base color (tint)
in vec4 v_add_color;
// binormal, tangent, normal matrix
in mat3 v_btn;
// eye-to-vertex in world space
in vec3 v_eye_ray;

// Environment map
uniform samplerCube u_tex0;
// Bump map
uniform sampler2D u_tex1;

out vec4 o_color;

void main()
{
	// Texture coordinates are used to sample bump map
	vec3 bump_sample = normalize(2.0 * texture(u_tex1, v_tex_coord).xyz - 1.0);
	// Convert normal from tangent to world space
	vec3 normal = normalize(v_btn * bump_sample);
	// Compute reflection and sample environment map
	vec3 refl = reflect(v_eye_ray, normal);
	vec4 env_sample = texture(u_tex0, refl);
	// tint + environment * mod_color
	o_color = vec4(env_sample.rgb * v_mod_color.rgb + v_add_color.rgb, v_mod_color.a);
}