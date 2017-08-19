#include "stdafx.h"
#include "mathutil.h"

namespace dukat
{
	float wrap_pi(float theta)
	{
		theta += pi;
		theta -= floor(theta * one_over_two_pi) * two_pi;
		theta -= pi;
		return theta;
	}

	float safe_acos(float x)
	{
		// check limit conditions
		if (x <= -1.0f)
		{
			return pi;
		}
		if (x >= 1.0f)
		{
			return 0.0f;
		}
		return acos(x);
	}

	float inv_sqrt(float val)
	{
		long i;
		float x2, y;

		x2 = val * 0.5f;
		y = val;
		i = *(long *)&y;
		i = 0x5f3759df - (i >> 1);
		y = *(float *)&i;
		y = y * (1.5f - (x2 * y * y));

		return y;
	}
}