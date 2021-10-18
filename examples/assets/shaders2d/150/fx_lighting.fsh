#version 150
///
// Lighting fragment shader
///
in vec2 v_tex_coord;

layout(std140) uniform Camera
{
	mat4 proj_orth;
	mat4 view;
    vec2 position;
	vec2 dimension;
} u_cam;

struct Light 
{
	vec2 position;
	float k1;
	float k2;
	vec4 color;
};

layout(std140) uniform Lights 
{
	Light l[16];
} u_lights;

// scene texture
uniform sampler2D u_tex0;

// aspect ratio
uniform float u_aspect;
// ambient light
uniform vec4 u_ambient;

out vec4 o_color;

vec4 compute_light(vec2 pos, Light light)
{
	float distance = length(pos - light.position);
	float attenuation = 1.0 / (1.0 + light.k1 * distance + light.k2 * (distance * distance));    
	return light.color * attenuation;
}

void main()
{
	// Sample base texture
	vec4 base_color = texture(u_tex0, v_tex_coord);
	
	// convert screen position into cam space 
	// denormalize [ from 0..1 to -0.5..0.5 ]
	vec2 screen_offset = vec2(v_tex_coord.x, 1 - v_tex_coord.y) - 0.5;
	// convert into screen space and add to absolute camera position
	vec2 pos = u_cam.position + screen_offset * u_cam.dimension;

	// compute contribution of each light source
	vec4 ambient = u_ambient;
	for (int i = 0; i < 16; i++)
		ambient += compute_light(pos, u_lights.l[i]);

	// Clamp value
	ambient = min(vec4(1,1,1,1), ambient);
	
	o_color = vec4(base_color.rgb * ambient.rgb, 1);
}
