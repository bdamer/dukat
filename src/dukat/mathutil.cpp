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
}