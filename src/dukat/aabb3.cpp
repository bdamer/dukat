#include "stdafx.h"
#include "aabb3.h"
#include "mathutil.h"
#include "matrix4.h"
#include "ray3.h"

namespace dukat
{
	AABB3::AABB3(const std::vector<Vector3>& vectors)
	{
		clear();
		for (auto& it : vectors)
		{
			add(it);
		}
	}

	void AABB3::clear()
	{
		min.x = min.y = min.z = big_number;
		max.x = max.y = max.z = -big_number;
	}

	void AABB3::add(const Vector3& p)
	{
		if (p.x < min.x)
		{
			min.x = p.x;
		}
		if (p.y < min.y)
		{
			min.y = p.y;
		}
		if (p.z < min.z)
		{
			min.z = p.z;
		}
		if (p.x > max.x)
		{
			max.x = p.x;
		}
		if (p.y > max.y)
		{
			max.y = p.y;
		}
		if (p.z > max.z)
		{
			max.z = p.z;
		}
	}

	void AABB3::add(const AABB3& box)
	{
		add(box.min);
		add(box.max);
	}

	void AABB3::set_to_transformed_box(const AABB3& box, const Matrix4& m)
	{
		// if we're empty, then bail
		if (box.empty())
		{
			clear();
			return;
		}
		// start with the translation portion
		min = max = { m[12], m[13], m[14] };

		// Examine each of the nine matrix elements and comute the new AABB
		if (m.m[0] > 0.0f)
		{
			min.x += m.m[0] * box.min.x;
			max.x += m.m[0] * box.max.x;
		}
		else
		{
			min.x += m.m[0] * box.max.x;
			max.x += m.m[0] * box.min.x;
		}
		if (m.m[1] > 0.0f)
		{
			min.y += m.m[1] * box.min.x;
			max.y += m.m[1] * box.max.x;
		}
		else
		{
			min.y += m.m[1] * box.max.x;
			max.y += m.m[1] * box.min.x;
		}
		if (m.m[2] > 0.0f)
		{
			min.z += m.m[2] * box.min.x;
			max.z += m.m[2] * box.max.x;
		}
		else
		{
			min.z += m.m[2] * box.max.x;
			max.z += m.m[2] * box.min.x;
		}
		if (m.m[4] > 0.0f)
		{
			min.x += m.m[4] * box.min.y;
			max.x += m.m[4] * box.max.y;
		}
		else
		{
			min.x += m.m[4] * box.max.y;
			max.x += m.m[4] * box.min.y;
		}
		if (m.m[5] > 0.0f)
		{
			min.y += m.m[5] * box.min.y;
			max.y += m.m[5] * box.max.y;
		}
		else
		{
			min.y += m.m[5] * box.max.y;
			max.y += m.m[5] * box.min.y;
		}
		if (m.m[6] > 0.0f)
		{
			min.z += m.m[6] * box.min.y;
			max.z += m.m[6] * box.max.y;
		}
		else
		{
			min.z += m.m[6] * box.max.y;
			max.z += m.m[6] * box.min.y;
		}
		if (m.m[8] > 0.0f)
		{
			min.x += m.m[8] * box.min.z;
			max.x += m.m[8] * box.max.z;
		}
		else
		{
			min.x += m.m[8] * box.max.z;
			max.x += m.m[8] * box.min.z;
		}
		if (m.m[9] > 0.0f)
		{
			min.y += m.m[9] * box.min.z;
			max.y += m.m[9] * box.max.z;
		}
		else
		{
			min.y += m.m[9] * box.max.z;
			max.y += m.m[9] * box.min.z;
		}
		if (m.m[10] > 0.0f)
		{
			min.z += m.m[10] * box.min.z;
			max.z += m.m[10] * box.max.z;
		}
		else
		{
			min.z += m.m[10] * box.max.z;
			max.z += m.m[10] * box.min.z;
		}
	}

    std::unique_ptr<BoundingBody3> AABB3::transform(const Matrix4 & m)
    {
        auto res = std::make_unique<AABB3>();
        res->set_to_transformed_box(*this, m);
        return std::move(res);
    }

	bool AABB3::empty(void) const 
	{
		// check if we're inverted on any axis
		return (min.x > max.x) || (min.y > max.y) || (min.z > max.z);
	}

	bool AABB3::contains(const Vector3& p) const 
	{
		// check for overlap on each axis
		return
			(p.x >= min.x) && (p.x <= max.x) &&
			(p.y >= min.y) && (p.y <= max.y) &&
			(p.z >= min.z) && (p.z <= max.z);
	}

    bool AABB3::intersect_sphere(const Vector3 & center, float radius) const
    {
        return false;
    }

	// from: http://www.scratchapixel.com/lessons/3d-basic-lessons/lesson-7-intersecting-simple-shapes/ray-box-intersection/
	// there are more optimizations that we can apply.
	float AABB3::intersect_ray(const Ray3& ray, float near, float far, Vector3* returnNormal) const 
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
		float tzmin = (min.z - ray.origin.z) / ray.dir.z;
		float tzmax = (max.z - ray.origin.z) / ray.dir.z;
		if (tzmin > tzmax) std::swap(tzmin, tzmax);
		if ((tmin > tzmax) || (tzmin > tmax))
			return no_intersection;
		if (tzmin > tmin)
			tmin = tzmin;
		if (tzmax < tmax)
			tmax = tzmax;
		if ((tmin > far) || (tmax < near)) return no_intersection;
		// these are the min/max t values
		//if (r.tmin < tmin) r.tmin = tmin;
		//if (r.tmax > tmax) r.tmax = tmax;
		//return true;
		return tmin;
	}

	bool AABB3::intersect_aabb(const AABB3& another) const
	{
		if (max.x < another.min.x) return false; // a is left of b
		if (min.x > another.max.x) return false; // a is right of b
		if (max.y < another.min.y) return false; // a is above b
		if (min.y > another.max.y) return false; // a is below b
		if (max.z < another.min.z) return false; // a is above b
		if (min.z > another.max.z) return false; // a is below b
		return true; // boxes overlap
	}
}