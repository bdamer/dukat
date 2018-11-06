#include "stdafx.h"
#include <dukat/boundingcircle.h>
#include <dukat/mathutil.h>

namespace dukat
{
	bool BoundingCircle::intersect_circle(const Vector2& center, float radius) const
	{
		auto dist = (this->center - center).mag();
		return (dist <= (this->radius + radius));
	}

	float BoundingCircle::intersect_ray(const Ray2& ray, float near_z, float far_z) const
	{
		const auto a = ray.dir * ray.dir;
		// Center to start of ray
		const auto f = ray.origin - center;
		const auto b = 2.0f * (f * ray.dir);
		const auto c = f * f - radius * radius;
		auto discriminant = b * b - 4 * a*c;
		if (discriminant < 0)
		{
			return no_intersection;
		}
		else
		{
			discriminant = std::sqrt(discriminant);
			auto t1 = (-b - discriminant) / (2 * a);
			auto t2 = (-b + discriminant) / (2 * a);
			if (t1 >= 0 && t1 <= 1)
				return t1;
			else if (t2 >= 0 && t2 <= 1)
				return t2;
			else 
				return no_intersection;
		}
	}
}