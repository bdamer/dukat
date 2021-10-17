#pragma once

#include "color.h"
#include "vector2.h"
#include "vector3.h"

namespace dukat
{
	// Light structure passed to shader.
	// 
	// Attenuation constants based on Ogre3D wiki:
	//	Range	k0 		k1 		k2
	//	7 		1.0 	0.7 	1.8
	//	13 		1.0 	0.35 	0.44
	//	20 		1.0 	0.22 	0.20
	//	32 		1.0 	0.14 	0.07
	//	50 		1.0 	0.09 	0.032
	//	65 		1.0 	0.07 	0.017
	//	100 	1.0 	0.045 	0.0075
	//	160 	1.0 	0.027 	0.0028
	//	200 	1.0 	0.022 	0.0019
	//	325 	1.0 	0.014 	0.0007
	//	600 	1.0 	0.007 	0.0002
	//	3250 	1.0 	0.0014 	0.000007
	struct Light3
	{
		Vector3 position;	// position (point light) or direction (directional light)
		Color ambient;		// ambient color
		Color diffuse;		// diffuse color
		Color specular;		// specular color
		GLfloat k0, k1, k2; // attenuation coefficients 
		GLfloat reserved1;	// padding
	};

	struct Light2
	{
		Vector2 position;	// position in world space
		GLfloat k1, k2;		// attenuation coefficients
		Color color;		// color
	};

	template<typename T>
	inline void set_light_range(T& light, float range) { light.k1 = 4.5f / range; light.k2 = 75.f / (range * range); }
}