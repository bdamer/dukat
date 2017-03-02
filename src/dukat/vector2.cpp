#include "stdafx.h"
#include "vector2.h"
#include "mathutil.h"

namespace dukat
{
	// Left-handed coordinate system
	const Vector2 Vector2::origin(0.0f, 0.0f);
	const Vector2 Vector2::unit_x(1.0f, 0.0f);
	const Vector2 Vector2::unit_y(0.0f, 1.0f);

	Vector2 Vector2::rotate(float theta) const
	{
		float cs = cos(theta);
		float sn = sin(theta);
		return Vector2(x * cs - y * sn, x * sn + y * cs);
	}

	Vector2& Vector2::normalize_fast(void)
	{
		auto mag_sq = x * x + y * y;
		if (mag_sq > 0.0f)
		{
			auto one_over_mag = inv_sqrt(mag_sq);
			x *= one_over_mag;
			y *= one_over_mag;
		}
		return *this;
	}

	float Vector2::angle_between(const Vector2& v) const
	{
		float angle = safe_acos(*this * v);
		// "direction" of angle - left if positive, otherwise right
		if (0 > (x * v.y - y * v.x)) 
		{
			angle = -angle;
		}
		return angle;
	}

	float Vector2::angle(void) const
	{
		// angle between this and (1,0)
		float angle = safe_acos(x);
		// "direction" of angle - left if positive, otherwise right
		if (y < 0) 
		{
			angle = -angle;
		}
		return angle;	
	}

	Vector2 Vector2::random(const Vector2& min, const Vector2& max)
	{
		return Vector2(randf(min.x, max.x), randf(min.y, max.y));
	}

	std::ostream& operator<<(std::ostream& os, const Vector2& v)
	{
		os << v.x << "," << v.y;
		return os;
	}
}