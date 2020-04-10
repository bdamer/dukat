#include "stdafx.h"
#include <dukat/vector3.h>
#include <dukat/mathutil.h>
#include <random>
#include <dukat/rand.h>

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
		const auto z = random(-mag, mag);
		const auto phi = random(0.0f, two_pi);
		const auto theta = std::asin(z / mag);
		return Vector3{
			mag * std::cos(theta) * std::cos(phi),
			mag * std::cos(theta) * std::sin(phi),
			z 
		};
	}

	void generate_distribution(std::vector<Vector3>& data, const Vector3& min_v, const Vector3& max_v)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> x_dist(min_v.x, max_v.x);
		std::uniform_real_distribution<float> y_dist(min_v.y, max_v.y);
		std::uniform_real_distribution<float> z_dist(min_v.z, max_v.z);
		for (auto& el : data)
		{
			el.x = x_dist(gen);
			el.y = y_dist(gen);
			el.z = z_dist(gen);
		}
	}

	std::ostream& operator<<(std::ostream& os, const Vector3& v)
	{
		os << "[" << v.x << "," << v.y << "," << v.z << "]";
		return os;
	}
}