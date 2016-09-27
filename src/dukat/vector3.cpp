#include "stdafx.h"
#include "vector3.h"
#include "mathutil.h"

namespace dukat
{
	// Left-handed coordinate system
	const Vector3 Vector3::origin(0, 0, 0);
	const Vector3 Vector3::unit_x(1.0f, 0.0f, 0.0f);
	const Vector3 Vector3::unit_y(0.0f, 1.0f, 0.0f);
	const Vector3 Vector3::unit_z(0.0f, 0.0f, 1.0f);

	Vector3& Vector3::normalize(void)
	{
		float mag_sq = x * x + y * y + z * z;
		if (mag_sq > 0.0f)
		{
			float one_over_mag = 1.0f / sqrt(mag_sq);
			x *= one_over_mag;
			y *= one_over_mag;
			z *= one_over_mag;
		}
		return *this;
	}	

	void Vector3::sanitize(void)
	{
		if (abs(x) < small_number)
		{
			x = 0.0f;
		}
		if (abs(y) < small_number)
		{
			y = 0.0f;
		}
		if (abs(z) < small_number)
		{
			z = 0.0f;
		}
	}

	void Vector3::randomize(float mag)
	{
		z = randf(-mag, mag);
		float phi = randf(0, two_pi);
		float theta = asin(z / mag);
		x = mag * cos(theta) * cos(phi);
		y = mag * cos(theta) * sin(phi);
	}

	std::ostream& operator<<(std::ostream& os, const Vector3& v)
	{
		os << "[" << v.x << "," << v.y << "," << v.z << "]";
		return os;
	}
}