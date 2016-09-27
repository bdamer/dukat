#pragma once

#include "boundingbody2.h"
#include "vector2.h"

namespace dukat
{
	class AABB2;
	struct Transform2;

	// 2D Oriented bounding box.
	class OBB2 : public BoundingBody2
	{
	private:
		// origin[i] = corner[0] dot axis[i]
		float origin[2];
		// Updates the axes after the corners move. Assumes the corners form a rectangle.
		void compute_axes(void);
		// Checks if the other OBB2 overlaps one dimension of this one.
		bool overlap_test(const OBB2& other) const;

	public:
		// Corners of the box from lower-left
		Vector2 corners[4];
		// Edges of box extended from lower-left corner.
		Vector2 axis[2];

		// Creates a new oriented box
		OBB2(void);
		OBB2(const Vector2& center, const Vector2& dimension, float rotation);
		OBB2(const AABB2& box, const Transform2& transform);
		~OBB2(void) { }

		// Box operations
		void set_to_transformed(const AABB2& box, const Transform2& transform);
		void clear(void);

		// Containment / Intersection tests
		bool empty(void) const;
		bool contains(const Vector2& p) const;
		bool intersect_circle(const Vector2& center, float radius) const;
		float intersect_ray(const Ray2& ray, float near, float far) const;
		// Checks if two OBB2s overlap.
		bool overlaps(const OBB2& other) const;
	};
}