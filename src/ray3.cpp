#include "stdafx.h"
#include "ray3.h"
#include "plane.h"

namespace dukat
{
	float Ray3::intersect_plane(const Plane &plane) const 
	{
		float denominator = dir * plane.n;
		if (denominator == 0.0f)
		{
			// no intersection, because ray is parallel to plane
			return 0.0f;
		}
		float numerator = plane.d - origin * plane.n;
		return numerator / denominator;
	}

	Ray3& Ray3::from_points(const Vector3& p1, const Vector3& p2)
	{
		origin = p1;
		dir = p2 - p1;
		dir.normalize();
		return *this;
	}
}