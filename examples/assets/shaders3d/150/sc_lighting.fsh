#version 150
///
// Textured and lit fragment shader.
///
in vec3 v_position;
in vec3 v_normal;
in vec2 v_tex_coord;

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

layout(std140) uniform Material
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float shininess;
	vec3 custom;
} u_material;

struct Light 
{
	vec4 position;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
    vec4 params;
};

layout(std140) uniform Lights 
{
	Light l[5];
} u_lights;

uniform sampler2D u_tex0;

out vec4 o_color;

#include "lib_phong.fsh"

void main()
{
	vec3 normal = normalize(v_normal.xyz);
	vec3 view_dir = normalize(u_cam.position.xyz - v_position);
	vec4 surface = texture(u_tex0, v_tex_coord);
	
	// Phase 1: directional light
	vec3 color = dir_light(u_lights.l[0], normal, view_dir, surface);
	// Phase 2: point lights
	color += point_light(u_lights.l[1], normal, v_position, view_dir, surface);    
	color += point_light(u_lights.l[2], normal, v_position, view_dir, surface);    
	color += point_light(u_lights.l[3], normal, v_position, view_dir, surface);    
	color += point_light(u_lights.l[4], normal, v_position, view_dir, surface);    
	
	o_color = vec4(color, surface.a);
}