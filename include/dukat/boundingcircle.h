#pragma once

#include "boundingbody2.h"
#include "ray2.h"
#include "vector2.h"

namespace dukat
{
	class BoundingCircle : public BoundingBody2
	{
	public:
		Vector2 center;
		float radius;

		BoundingCircle(const Vector2& center, float radius) : center(center), radius(radius) { }
		~BoundingCircle(void) { }

		// Containment / Intersection tests
		bool contains(const Vector2& p) const { return (p - center).mag() <= radius; }
		// Checks if body intersects circle.
		bool intersect_circle(const Vector2& center, float radius) const;
		// Checkfs if body intersects ray. Will return the distance along intersection ray or no_intersection.
		float intersect_ray(const Ray2& ray, float near_z, float far_z) const;
	};
}