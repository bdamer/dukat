#include "stdafx.h"
#include <dukat/vector3.h>
#include <dukat/mathutil.h>

namespace dukat
{
	// Left-handed coordinate system
	const Vector3 Vector3::origin(0, 0, 0);
	const Vector3 Vector3::unit_x(1.0f, 0.0f, 0.0f);
	const Vector3 Vector3::unit_y(0.0f, 1.0f, 0.0f);
	const Vector3 Vector3::unit_z(0.0f, 0.0f, 1.0f);

	Vector3& Vector3::normalize(void)
	{
		const auto mag_sq = x * x + y * y + z * z;
		if (mag_sq > 0.0f)
		{
			const auto one_over_mag = 1.0f / std::sqrt(mag_sq);
			x *= one_over_mag;
			y *= one_over_mag;
			z *= one_over_mag;
		}
		return *this;
	}

	Vector3& Vector3::normalize_fast(void)
	{
		const auto mag_sq = x * x + y * y + z * z;
		if (mag_sq > 0.0f)
		{
			const auto one_over_mag = inv_sqrt(mag_sq);
			x *= one_over_mag;
			y *= one_over_mag;
			z *= one_over_mag;
		}
		return *this;
	}

	void Vector3::sanitize(void)
	{
		if (std::abs(x) < small_number)
			x = 0.0f;
		if (std::abs(y) < small_number)
			y = 0.0f;
		if (std::abs(z) < small_number)
			z = 0.0f;
	}

	Vector3 random(float mag)
	{
		const auto z = randf(-mag, mag);
		const auto phi = randf(0, two_pi);
		const auto theta = std::asin(z / mag);
		return Vector3{
			mag * std::cos(theta) * std::cos(phi),
			mag * std::cos(theta) * std::sin(phi),
			z 
		};
	}

	std::ostream& operator<<(std::ostream& os, const Vector3& v)
	{
		os << "[" << v.x << "," << v.y << "," << v.z << "]";
		return os;
	}
}