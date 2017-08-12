/// 
// Phong lighting model implementation.
///
//
// Uniforms:
// u_material
//

// Compute directional light.
// light - Directional light
// normal - Normalized fragment normal in camera space.
// view_dir - Normalized vector from camera to fragment.
// color - Fragment color
vec3 dir_light(gl_LightSourceParameters light, vec3 normal, vec3 view_dir, vec4 color)
{
	vec3 light_dir = normalize(-light.position.xyz);
    // diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);
    // specular shading
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = 0.0;
	if (diff > 0.0)
	{
		spec = pow(max(dot(view_dir, reflect_dir), 0.0), gl_FrontMaterial.shininess);
	}	
    // combine results
    vec3 ambient = light.ambient.rgb * gl_FrontMaterial.ambient.rgb * color.rgb;
    vec3 diffuse = light.diffuse.rgb * diff * gl_FrontMaterial.diffuse.rgb * color.rgb;
	vec3 specular = light.specular.rgb * spec * gl_FrontMaterial.specular.rgb * color.rgb;
	return (ambient + diffuse + specular);
}

// Compute point light.
// light - Point light
// normal - Normalized fragment normal in camera space.
// frag_pos - Fragment position in camera space.
// view_dir - Normalized vector from camera to fragment.
// color - Fragment color
vec3 point_light(gl_LightSourceParameters light, vec3 normal, vec3 frag_pos, vec3 view_dir, vec4 color)
{
	vec3 light_dir = normalize(light.position.xyz - frag_pos);
    // diffuse shading
    float diff = max(dot(normal, light_dir), 0.0);
    // specular shading
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = 0.0;
	if (diff > 0.0)
	{
		spec = pow(max(dot(view_dir, reflect_dir), 0.0), gl_FrontMaterial.shininess);
	}	
    // attenuation
    float distance = length(light.position.xyz - frag_pos);
    float attenuation = 1.0 / (light.constantAttenuation + light.linearAttenuation * distance + light.quadraticAttenuation * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient.rgb * gl_FrontMaterial.ambient.rgb * color.rgb;
    vec3 diffuse = light.diffuse.rgb * diff * gl_FrontMaterial.diffuse.rgb * color.rgb;
	vec3 specular = light.specular.rgb * spec * gl_FrontMaterial.specular.rgb * color.rgb;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}