#include "stdafx.h"
#include "aabb2.h"
#include "mathutil.h"
#include "matrix2.h"

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
		auto tmin = (min.x - ray.origin.x) / ray.dir.x;
		auto tmax = (max.x - ray.origin.x) / ray.dir.x;
		if (tmin > tmax) 
			std::swap(tmin, tmax);
		auto tymin = (min.y - ray.origin.y) / ray.dir.y;
		auto tymax = (max.y - ray.origin.y) / ray.dir.y;
		if (tymin > tymax) 
			std::swap(tymin, tymax);
		if ((tmin > tymax) || (tymin > tmax))
			return no_intersection;
		if (tymin > tmin)
			tmin = tymin;
		if (tymax < tmax)
			tmax = tymax;
		if ((tmin > far) || (tmax < near)) 
			return no_intersection;
		return tmin;
	}

	int AABB2::classify_ray(const Ray2& ray) const
	{
		// compute ray normal by rotating the direction by 90 degrees CW
		// this means that the "right" side of the ray is considered its 
		// front side 
		Vector2 n(-ray.dir.y, ray.dir.x);
		auto d = ray.origin * n;
		
		// Inspect the normal and compute the minimum and 
		// maximum D values
		float mind, maxd;

		if (n.x > 0.0f)
		{
			mind = n.x * min.x;
			maxd = n.x * max.x;
		}
		else
		{
			mind = n.x * max.x;
			maxd = n.x * min.x;
		}

		if (n.y > 0.0f)
		{
			mind += n.y * min.y;
			maxd += n.y * max.y;
		}
		else
		{
			mind += n.y * max.y;
			maxd += n.y * min.y;
		}

		// check if completely on the front side of the plane
		if (mind >= d)
		{
			return 1;
		}
		// check if completely on the back side of plane
		else if (maxd <= d)
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}
}