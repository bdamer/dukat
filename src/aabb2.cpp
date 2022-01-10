#include "stdafx.h"
#include <dukat/aabb2.h>
#include <dukat/mathutil.h>
#include <dukat/matrix2.h>

namespace dukat
{
	bool AABB2::overlaps(const AABB2& another) const
	{
		// Using exclusive check so adjacent BBs don't overlap
		if (_max.x <= another._min.x) return false; // a is left of b
		if (_min.x >= another._max.x) return false; // a is right of b
		if (_max.y <= another._min.y) return false; // a is above b
		if (_min.y >= another._max.y) return false; // a is below b
		return true; // boxes overlap
	}

	bool AABB2::overlaps(const BoundingBody2& another) const
	{
		if (auto ptr = dynamic_cast<const AABB2*>(&another))
			return overlaps(*ptr);
		else if (auto ptr = dynamic_cast<const BoundingCircle*>(&another))
			return intersect_circle(*ptr);
		else 
			return false;
	}

	bool AABB2::intersect(const AABB2& another, Collision& collision) const
	{
		const auto this_c = center();
		const auto that_c = another.center();
		const auto d = this_c - that_c;
		const auto p = ((_max - _min) + (another._max - another._min)) * 0.5f - abs(d);
		if (p.x <= 0.0f || p.y <= 0.0f)
			return false;

		if (p.x < p.y)
		{
			const auto sx = sgn(d.x);
			collision.delta.x = p.x * sx;
			collision.normal.x = static_cast<float>(sx);
			collision.pos.x = sx < 0 ? _min.x : _max.x;
			collision.pos.y = that_c.y;
		}
		else
		{
			const auto sy = sgn(d.y);
			collision.delta.y = p.y * sy;
			collision.normal.y = static_cast<float>(sy);
			collision.pos.x = that_c.x;			
			collision.pos.y = sy < 0 ? _min.y : _max.y;
		}
		return true;
	}

	bool AABB2::contains(const Vector2& p) const
	{
		// check for overlap on each axis
		return
			(p.x >= _min.x) && (p.x <= _max.x) &&
			(p.y >= _min.y) && (p.y <= _max.y);
	}

	bool AABB2::intersect_circle(const BoundingCircle& bc) const
	{
		auto v = _center - bc.center;
		const auto dist2_centers = v.mag2();

		// compute inner & outer radius of bounding box
		auto outer_rad = (_center - _min).mag2();
		auto inner_rad = (_max - _center).mag2();
		if (outer_rad < inner_rad)
			std::swap(outer_rad, inner_rad);

		// Case #1 - bb definitely outside of bb
		if (dist2_centers > outer_rad + bc.radius * bc.radius)
			return false;
		// Case #2 - bb definitely inside of bb
		if (dist2_centers < inner_rad + bc.radius * bc.radius)
			return true;
		// Case #3 - test point on circle on vector between centers
		v.normalize();
		return contains(_center + v * bc.radius);
	}

	void AABB2::clear()
	{
		_min.x = _min.y = big_number;
		_max.x = _max.y = -big_number;
	}

	void AABB2::add(const Vector2& p)
	{
		if (p.x < _min.x)
			_min.x = p.x;
		if (p.y < _min.y)
			_min.y = p.y;
		if (p.x > _max.x)
			_max.x = p.x;
		if (p.y > _max.y)
			_max.y = p.y;
		compute_center();
	}

	void AABB2::add(const AABB2& box)
	{
		add(box._min);
		add(box._max);
	}

	bool AABB2::empty(void) const
	{
		// check if we're inverted on any axis
		return (_min.x > _max.x) || (_min.y > _max.y);
	}

	void AABB2::set_to_transformed_box(const AABB2& box, const Transform2& t)
	{
		clear();
		add(t.pos + box._min.rotate(t.rot));
		add(t.pos + box._max.rotate(t.rot));
	}

	float AABB2::intersect_ray(const Ray2& ray, float near_z, float far_z) const 
	{
		auto tmin = (_min.x - ray.origin.x) / ray.dir.x;
		auto tmax = (_max.x - ray.origin.x) / ray.dir.x;
		if (tmin > tmax) 
			std::swap(tmin, tmax);
		auto tymin = (_min.y - ray.origin.y) / ray.dir.y;
		auto tymax = (_max.y - ray.origin.y) / ray.dir.y;
		if (tymin > tymax) 
			std::swap(tymin, tymax);
		if ((tmin > tymax) || (tymin > tmax))
			return no_intersection;
		if (tymin > tmin)
			tmin = tymin;
		if (tymax < tmax)
			tmax = tymax;
		if ((tmin > far_z) || (tmax < near_z))
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
			mind = n.x * _min.x;
			maxd = n.x * _max.x;
		}
		else
		{
			mind = n.x * _max.x;
			maxd = n.x * _min.x;
		}

		if (n.y > 0.0f)
		{
			mind += n.y * _min.y;
			maxd += n.y * _max.y;
		}
		else
		{
			mind += n.y * _max.y;
			maxd += n.y * _min.y;
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