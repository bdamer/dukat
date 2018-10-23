#pragma once

#include <memory>

namespace dukat
{
	class Ray2;
	class Vector2;

	class BoundingBody2
	{
	public:
		BoundingBody2(void) { }
		virtual ~BoundingBody2(void) { }

		// Containment / Intersection tests
		virtual bool contains(const Vector2& p) const = 0;
		// Checks if body intersects circle.
		virtual bool intersect_circle(const Vector2& center, float radius) const = 0;
		// Checks if body intersects ray. Will return the distance along intersection ray or no_intersection.
		virtual float intersect_ray(const Ray2& ray, float near, float far) const = 0;
	};
}