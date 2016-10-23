#pragma once

#include "boundingbody2.h"
#include "vector2.h"
#include "transform2.h"
#include "ray2.h"

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

		// Box operations
		void clear(void);
		void add(const Vector2& p);
		void add(const AABB2& box);

		// Sets this box to a transformed existing box.
		void set_to_transformed_box(const AABB2& box, const Transform2& t);

		// Containment / Intersection tests
		bool empty(void) const;
		bool overlaps(const AABB2& another) const;
		bool contains(const Vector2& p) const;
		bool intersect_circle(const Vector2& center, float radius) const;
		float intersect_ray(const Ray2& ray, float near, float far) const;

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