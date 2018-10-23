#include "stdafx.h"
#include <dukat/mathutil.h>

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

	// Lookup table used for fast trig functions below
	struct TrigTable
	{
		float values[360];

		template<typename F>
		TrigTable(F f)
		{
			for (auto i = 0; i < 360; i++)
				values[i] = f(deg_to_rad(static_cast<float>(i)));
		}
	};

	float fast_sin(float value)
	{
		static TrigTable table(sinf);
		auto deg = static_cast<int>(rad_to_deg(value));
		return table.values[deg % 360];
	}

	float fast_cos(float value)
	{
		static TrigTable table(cosf);
		auto deg = static_cast<int>(rad_to_deg(value));
		return table.values[deg % 360];
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