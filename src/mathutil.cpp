#include "stdafx.h"
#include <dukat/mathutil.h>
#include <array>

namespace dukat
{
	// Lookup table used for fast trig functions below
	struct TrigTable
	{
		static constexpr auto size = 2048;
		// Store values between 0 and pi / 2
		std::array<float, size> table;

		template<typename F>
		TrigTable(F f)
		{
			for (auto i = 0; i < size; i++)
				table[i] = f(i * pi / static_cast<float>(size * 2));
		}

		float lookup(float theta)
		{
			const auto angle = wrap_two_pi(theta);
			if (angle < pi_over_two) // Q1
				return table[static_cast<int>(angle * (size - 1) / pi_over_two)];
			else if (angle < pi) // Q2
				return table[static_cast<int>((pi - angle) * (size - 1) / pi_over_two)];
			else if (angle < 3.f * pi_over_two) // Q3
				return -table[static_cast<int>((angle - pi) * (size - 1) / pi_over_two)];
			else // Q4
				return -table[static_cast<int>((two_pi - angle) * (size - 1) / pi_over_two)];
		}
	};

	static TrigTable table(sinf);

	float fast_sin(float theta)
	{
		return table.lookup(theta);
	}

	float fast_cos(float theta)
	{
		return table.lookup(pi_over_two - theta);
	}

	float wrap_pi(float theta)
	{
		theta += pi;
		theta -= std::floor(theta * one_over_two_pi) * two_pi;
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
		return std::acos(x);
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