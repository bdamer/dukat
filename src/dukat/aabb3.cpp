#include "stdafx.h"
#include "aabb3.h"
#include "mathutil.h"
#include "matrix4.h"
#include "plane.h"
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

	float AABB3::intersect_ray(const Ray3& ray, float near, float far, Vector3* returnNormal) const 
	{
		auto r_inv = ray.dir.inverse();
		auto tx1 = (min.x - ray.origin.x) * r_inv.x;
		auto tx2 = (max.x - ray.origin.x) * r_inv.x;
		auto tmin = std::min(tx1, tx2);
		auto tmax = std::max(tx1, tx2);

		auto ty1 = (min.y - ray.origin.y) * r_inv.y;
		auto ty2 = (max.y - ray.origin.y) * r_inv.y;
		tmin = std::max(tmin, std::min(ty1, ty2));
		tmax = std::min(tmax, std::max(ty1, ty2));

		auto tz1 = (min.z - ray.origin.z) * r_inv.z;
		auto tz2 = (max.z - ray.origin.z) * r_inv.z;
		tmin = std::max(tmin, std::min(tz1, tz2));
		tmax = std::min(tmax, std::max(tz1, tz2));

		if ((tmax < tmin) || ((tmin > far) || (tmax < near)))
		{
			return no_intersection;
		}
		else
		{
			return tmin;
		}
	}

	int AABB3::classify_plane(const Plane& p) const
	{
		// Inspect the normal and compute the minimum and 
		// maximum D values
		float mind, maxd;

		if (p.n.x > 0.0f)
		{
			mind = p.n.x * min.x;
			maxd = p.n.x * max.x;
		}
		else
		{
			mind = p.n.x * max.x;
			maxd = p.n.x * min.x;
		}

		if (p.n.y > 0.0f)
		{
			mind += p.n.y * min.y;
			maxd += p.n.y * max.y;
		}
		else
		{
			mind += p.n.y * max.y;
			maxd += p.n.y * min.y;
		}

		if (p.n.z > 0.0f)
		{
			mind += p.n.z * min.z;
			maxd += p.n.z * max.z;
		}
		else
		{
			mind += p.n.z * max.z;
			maxd += p.n.z * min.z;
		}

		// check if completely on the front side of the plane
		if (mind >= p.d)
		{
			return 1;
		}
		// check if completely on the back side of plane
		else if (maxd <= p.d)
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}

	float AABB3::intersect_plane(const Plane& p) const
	{
		// compute glancing angle. Make sure we are moving towards
		// the front of the plane.
		/* TODO: fixme
		auto dot = p.n * dir;
		if (dot >= 0.0f)
		{
			return no_intersection;
		}*/

		// inspect the normal and compute the minimum and maximum
		// D values. minD is the D value of the "frontmost" corner point
		float mind, maxd;
		if (p.n.x > 0.0f)
		{
			mind = p.n.x * min.x;
			maxd = p.n.x * max.x;
		}
		else
		{
			mind = p.n.x * max.x;
			maxd = p.n.x * min.x;
		}

		if (p.n.y > 0.0f)
		{
			mind += p.n.y * min.y;
			maxd += p.n.y * max.y;
		}
		else
		{
			mind += p.n.y * max.y;
			maxd += p.n.y * min.y;
		}

		if (p.n.z > 0.0f)
		{
			mind += p.n.z * min.z;
			maxd += p.n.z * max.z;
		}
		else
		{
			mind += p.n.z * max.z;
			maxd += p.n.z * min.z;
		}

		// check if we're already completely on the other
		// side of the plane
		if (maxd <= p.d)
		{
			return no_intersection;
		}

		// perform standard ray trace equation using the
		// frontmost corner point
		auto t = (p.d - mind) / p.d;

		// Were we already penetrating?
		if (t < 0.0f)
		{
			return 0.0f;
		}
		// Return it. If > 1, then we didnt hit in time. That's the 
		// condition that the caller should be checking for.
		else 
		{
			return t;
		}
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