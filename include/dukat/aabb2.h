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
	private:
		Vector2 _min;
		Vector2 _max;
		Vector2 _center;
		void compute_center(void) { _center = _min + (_max - _min) * 0.5f; }

	public:
		// Constructors
		AABB2(void) { clear(); }
		AABB2(const Vector2& min, const Vector2& max) : _min(min), _max(max) { compute_center(); }

		// Dimension queries
		float size(void) const { return (_max.x - _min.x) * (_max.y - _min.y); }
		const Vector2& min(void) const { return _min; }
		const Vector2& max(void) const { return _max; }
		const Vector2& center(void) const { return _center; }
		float width(void) const { return _max.x - _min.x; }
		float height(void) const { return _max.y - _min.y; }

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
		AABB2 operator+(const Vector2& v) const { return AABB2(_min + v, _max + v); }
		AABB2 operator-(const Vector2& v) const { return AABB2(_min - v, _max - v); }
		AABB2 operator*(float s) const { return AABB2(_min * s, _max * s); }
		AABB2 operator/(float s) const { return AABB2(_min / s, _max / s); }

		friend AABB2& operator+=(AABB2& bb, const Vector2& v);
		friend AABB2& operator-=(AABB2& bb, const Vector2& v);
		friend AABB2& operator*=(AABB2& bb, float s);
		friend AABB2& operator/=(AABB2& bb, float s);
	};

	inline AABB2& operator+=(AABB2& bb, const Vector2& v)
	{
		bb._min += v;
		bb._max += v;
		bb.compute_center();
		return bb;
	}

	inline AABB2& operator-=(AABB2& bb, const Vector2& v)
	{
		bb._min -= v;
		bb._max -= v;
		bb.compute_center();
		return bb;
	}

	inline AABB2& operator*=(AABB2& bb, float s)
	{
		bb._min *= s;
		bb._max *= s;
		bb.compute_center();
		return bb;
	}

	inline AABB2& operator/=(AABB2& bb, float s)
	{
		bb._min /= s;
		bb._max /= s;
		bb.compute_center();
		return bb;
	}
}