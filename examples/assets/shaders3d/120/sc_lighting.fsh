#version 120
///
// Textured and lit fragment shader.
///
varying vec3 v_position;
varying vec3 v_normal;

uniform mat4 u_cam_proj_pers;
uniform mat4 u_cam_proj_orth;
uniform mat4 u_cam_view;
uniform mat4 u_cam_view_inv;
uniform vec4 u_cam_position;
uniform vec4 u_cam_dir;
uniform vec4 u_cam_up;
uniform vec4 u_cam_left;

uniform sampler2D u_tex0;

#include "lib_phong.fsh"

void main()
{
	vec3 normal = normalize(v_normal.xyz);
	vec3 view_dir = normalize(u_cam_position.xyz - v_position);
	vec4 surface = texture2D(u_tex0, gl_TexCoord[0].st);
	
	// Phase 1: directional light
	vec3 color = dir_light(gl_LightSource[0], normal, view_dir, surface);
	// Phase 2: point lights
	color += point_light(gl_LightSource[1], normal, v_position, view_dir, surface);    
	color += point_light(gl_LightSource[2], normal, v_position, view_dir, surface);    
	color += point_light(gl_LightSource[3], normal, v_position, view_dir, surface);    
	color += point_light(gl_LightSource[4], normal, v_position, view_dir, surface);    
	
	gl_FragColor = vec4(color, surface.a);
}
