#include "stdafx.h"
#include <dukat/matrix2.h>
#include <dukat/vector2.h>

namespace dukat
{
	void Matrix2::identity(void)
	{
		m[0] = 1.0f;
		m[1] = 0.0f;
		m[2] = 0.0f;
		m[3] = 1.0f;
	}

	void Matrix2::setup_rotation(float angle)
	{
		m[0] = cos(angle);
		m[1] = sin(angle);
		m[2] = -m[1];
		m[3] = m[0];
	}

	Vector2 operator*(const Vector2& p, const Matrix2& m)
	{
		return Vector2(
			p.x*m.m[0] + p.y*m.m[2],
			p.x*m.m[1] + p.y*m.m[3]
		);
	}

}