#include "stdafx.h"
#include "matrix2.h"

namespace dukat
{
	void Matrix2::setup_rotation(float angle)
	{
		m[0] = cos(angle);
		m[1] = -sin(angle);
		m[2] = -m[1];
		m[3] = m[0];
	}
}