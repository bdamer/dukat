#include "stdafx.h"
#include "aabb2.h"
#include "mathutil.h"

namespace dukat
{
	bool AABB2::overlaps(const AABB2& another) const
	{
		if (max.x < another.min.x) return false; // a is left of b
		if (min.x > another.max.x) return false; // a is right of b
		if (max.y < another.min.y) return false; // a is above b
		if (min.y > another.max.y) return false; // a is below b
		return true; // boxes overlap
	}

	bool AABB2::contains(const Vector2& p) const
	{
		// check for overlap on each axis
		return
			(p.x >= min.x) && (p.x <= max.x) &&
			(p.y >= min.y) && (p.y <= max.y);
	}

	bool AABB2::intersect_circle(const Vector2 & center, float radius) const
	{
		// TODO: implement
		return false;
	}

	void AABB2::clear()
	{
		min.x = min.y = big_number;
		max.x = max.y = -big_number;
	}

	void AABB2::add(const Vector2& p)
	{
		if (p.x < min.x)
		{
			min.x = p.x;
		}
		if (p.y < min.y)
		{
			min.y = p.y;
		}
		if (p.x > max.x)
		{
			max.x = p.x;
		}
		if (p.y > max.y)
		{
			max.y = p.y;
		}
	}

	void AABB2::add(const AABB2& box)
	{
		add(box.min);
		add(box.max);
	}

	bool AABB2::empty(void) const
	{
		// check if we're inverted on any axis
		return (min.x > max.x) || (min.y > max.y);
	}

	void AABB2::set_to_transformed_box(const AABB2& box, const Transform2& t)
	{
		clear();
		add(t.pos + box.min.rotate(t.rot));
		add(t.pos + box.max.rotate(t.rot));
	}

	float AABB2::intersect_ray(const Ray2& ray, float near, float far) const 
	{
		float tmin = (min.x - ray.origin.x) / ray.dir.x;
		float tmax = (max.x - ray.origin.x) / ray.dir.x;
		if (tmin > tmax) std::swap(tmin, tmax);
		float tymin = (min.y - ray.origin.y) / ray.dir.y;
		float tymax = (max.y - ray.origin.y) / ray.dir.y;
		if (tymin > tymax) std::swap(tymin, tymax);
		if ((tmin > tymax) || (tymin > tmax))
			return no_intersection;
		if (tymin > tmin)
			tmin = tymin;
		if (tymax < tmax)
			tmax = tymax;
		if ((tmin > far) || (tmax < near)) return no_intersection;
		return tmin;
	}
}