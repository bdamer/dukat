#include "stdafx.h"
#include <dukat/vector2.h>
#include <random>

namespace dukat
{
	// Left-handed coordinate system
	const Vector2 Vector2::origin(0.0f, 0.0f);
	const Vector2 Vector2::unit_x(1.0f, 0.0f);
	const Vector2 Vector2::unit_y(0.0f, 1.0f);

	Vector2 Vector2::rotate(float theta) const
	{
		const auto cs = std::cos(theta);
		const auto sn = std::sin(theta);
		return Vector2(x * cs - y * sn, x * sn + y * cs);
	}

	Vector2& Vector2::normalize_fast(void)
	{
		const auto mag_sq = x * x + y * y;
		if (mag_sq > 0.0f)
		{
			const auto one_over_mag = inv_sqrt(mag_sq);
			x *= one_over_mag;
			y *= one_over_mag;
		}
		return *this;
	}

	float Vector2::angle(void) const
	{
		// angle between this and (1,0)
		const auto angle = safe_acos(x);
		// "direction" of angle - left if positive, otherwise right
		return (y < 0) ? -angle : angle;
	}

	float compute_angle(const Vector2& a, const Vector2& b)
	{
		const auto angle = safe_acos(a * b);
		// "direction" of angle - left if positive, otherwise right
		return (0 > (a.x * b.y - a.y * b.x)) ? -angle : angle;
	}

	void generate_distribution(std::vector<Vector2>& data, const Vector2& min_v, const Vector2& max_v)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> x_dist(min_v.x, max_v.x);
		std::uniform_real_distribution<float> y_dist(min_v.y, max_v.y);
		for (auto& el : data)
		{
			el.x = x_dist(gen);
			el.y = y_dist(gen);
		}
	}

	std::ostream& operator<<(std::ostream& os, const Vector2& v)
	{
		os << v.x << "," << v.y;
		return os;
	}
}