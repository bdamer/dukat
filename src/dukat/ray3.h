#pragma once

#include "vector3.h"

namespace dukat
{
	class Plane;

	class Ray3
	{
	public:
		Vector3 origin;
		Vector3 dir;	
		Ray3() { };
		Ray3(const Vector3& o, const Vector3& d) : origin(o), dir(d) { };
		Vector3 point_at(float t) const { return origin + dir * t; }
		// Checks if this ray intersects a plane.
		// Returns 0 if no intersection, otherwise the value
		// represents a value t where:
		// point of intersection = ray.origin + t * ray.dir 
		float intersect_plane(const Plane& p) const;
		// Initializes this ray so that origin = p1 and dir = p2 - p1.
		Ray3& from_points(const Vector3& p1, const Vector3& p2);
	};
}
