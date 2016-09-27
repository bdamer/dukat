#include "stdafx.h"
#include "obb2.h"
#include "aabb2.h"
#include "mathutil.h"

namespace dukat
{
	OBB2::OBB2(void)
	{
		clear();
	}

	OBB2::OBB2(const Vector2& center, const Vector2& dimension, float rotation)
	{
		Vector2 x(cos(rotation), sin(rotation));
		Vector2 y(-sin(rotation), cos(rotation));

		x *= dimension.x / 2.0f;
		y *= dimension.x / 2.0f;

		corners[0] = center - x - y;
		corners[1] = center + x - y;
		corners[2] = center + x + y;
		corners[3] = center - x + y;

		compute_axes();
	}

	OBB2::OBB2(const AABB2& box, const Transform2& transform)
	{
		set_to_transformed(box, transform);
	}

	void OBB2::set_to_transformed(const AABB2& box, const Transform2& transform)
	{
		Vector2 x(cos(transform.rot), sin(transform.rot));
		Vector2 y(-sin(transform.rot), cos(transform.rot));

		auto half_dim = (box.max - box.min) * 0.5f;
		x *= half_dim.x;
		y *= half_dim.y;
		auto center = box.min + half_dim;

		corners[0] = transform.pos + center - x - y;
		corners[1] = transform.pos + center + x - y;
		corners[2] = transform.pos + center + x + y;
		corners[3] = transform.pos + center - x + y;

		compute_axes();
	}

	void OBB2::clear(void)
	{
		for (int i = 0; i < 4; i++)
			corners[i] = Vector2(0, 0);
		for (int i = 0; i < 2; i++)
			axis[i] = Vector2(0, 0);
	}

	bool OBB2::empty(void) const
	{
		return axis[0].x == 0.0f && axis[0].y == 0.0f;
	}

	bool OBB2::contains(const Vector2 & p) const
	{
		// TODO: implement
		return false;
	}

	bool OBB2::intersect_circle(const Vector2 & center, float radius) const
	{
		// TODO: implement
		return false;
	}

	float OBB2::intersect_ray(const Ray2 & ray, float near, float) const
	{
		// TODO: implement
		return no_intersection;
	}

	void OBB2::compute_axes(void)
	{
		axis[0] = corners[1] - corners[0];
		axis[1] = corners[3] - corners[0];

		// Make the length of each axis 1/edge length so we know any
		// dot product must be less than 1 to fall within the edge.
		for (int a = 0; a < 2; ++a)
		{
			axis[a] /= axis[a].mag2();
			origin[a] = corners[0] * axis[a];
		}
	}

	bool OBB2::overlap_test(const OBB2& other) const
	{
		for (int a = 0; a < 2; ++a) 
		{
			float t = other.corners[0] * axis[a];

			// Find the extent of box 2 on axis a
			float tmin = t;
			float tmax = t;

			for (int c = 1; c < 4; ++c) 
			{
				t = other.corners[c] * axis[a];
				if (t < tmin)
				{
					tmin = t;
				}
				else if (t > tmax)
				{
					tmax = t;
				}
			}

			// We have to subtract off the origin

			// See if [tMin, tMax] intersects [0, 1]
			if ((tmin > 1 + origin[a]) || (tmax < origin[a])) 
			{
				// There was no intersection along this dimension;
				// the boxes cannot possibly overlap.
				return false;
			}
		}

		// There was no dimension along which there is no intersection.
		// Therefore the boxes overlap.
		return true;
	}

	bool OBB2::overlaps(const OBB2 & other) const
	{
		return overlap_test(other) && other.overlap_test(*this);
	}	
}