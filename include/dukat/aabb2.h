#pragma once

#include "boundingbody2.h"
#include "vector2.h"
#include "transform2.h"
#include "ray2.h"
#include "collision.h"
#include "boundingcircle.h"

namespace dukat
{
	// 2D Axis-aligned bounding body.
	class AABB2 : public BoundingBody2
	{
	public:
		Vector2 min;
		Vector2 max;

		// Constructors
		AABB2(void) { clear(); }
		AABB2(const Vector2& min, const Vector2& max) : min(min), max(max) { }

		// Dimension queries
		float size(void) const { return (max.x - min.x) * (max.y - min.y); }
		Vector2 center(void) const { return min + (max - min) * 0.5f; }
		float width(void) const { return max.x - min.x; }
		float height(void) const { return max.y - min.y; }

		// Box operations
		void clear(void);
		void add(const Vector2& p);
		void add(const AABB2& box);

		// Sets this box to a transformed existing box.
		void set_to_transformed_box(const AABB2& box, const Transform2& t);

		// Containment / Intersection tests
		bool empty(void) const;
		bool overlaps(const AABB2& another) const;
		bool overlaps(const BoundingBody2& another) const;
		bool intersect(const AABB2& another, Collision& collision) const;
		bool contains(const Vector2& p) const;
		bool intersect_circle(const BoundingCircle& bc) const;
		bool intersect_circle(const Vector2& center, float radius) const { return intersect_circle(BoundingCircle{ center, radius }); }
		float intersect_ray(const Ray2& ray, float near_z, float far_z) const;
		// Classifies box as being on one side or other other of a ray. 
		// Will return < 0 if box is completely on the left side of the ray
		// Will return > 0 if box is completely on the right side of the ray
		// Will return 0 if the box is intersected by the ray
		int classify_ray(const Ray2& ray) const;

		// Operators
		AABB2 operator+(const Vector2& v) const { return AABB2(min + v, max + v); }
		AABB2 operator-(const Vector2& v) const { return AABB2(min - v, max - v); }
		AABB2 operator*(float s) const { return AABB2(min * s, max * s); }
		AABB2 operator/(float s) const { return AABB2(min / s, max / s); }
	};

	inline AABB2& operator+=(AABB2& bb, const Vector2& v)
	{
		bb.min += v;
		bb.max += v;
		return bb;
	}

	inline AABB2& operator-=(AABB2& bb, const Vector2& v)
	{
		bb.min -= v;
		bb.max -= v;
		return bb;
	}

	inline AABB2& operator*=(AABB2& bb, float s)
	{
		bb.min *= s;
		bb.max *= s;
		return bb;
	}

	inline AABB2& operator/=(AABB2& bb, float s)
	{
		bb.min /= s;
		bb.max /= s;
		return bb;
	}
}