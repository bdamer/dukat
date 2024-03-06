#pragma once

#include "vector3.h"

namespace dukat
{
	struct Plane
	{
		Vector3 p;	// point on the plane
		Vector3 n;	// normal vector
		float d; // p * n = d
		Plane(void) : p(), n(), d(0.f) { };
		Plane(const Vector3 &point, const Vector3 &normal) : p(point), n(normal) { d = p * n; }; 
	};
}