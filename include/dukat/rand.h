#pragma once

#include <cstdlib>

namespace dukat
{
	inline float random(float min, float max)
	{
		if (min == max)
			return min;
		else
			return min + (max - min) * (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX));
	}

	inline int random(int min, int max)
	{
		if (min == max) // protect against / 0
			return min;
		else
			return min + std::rand() % (max - min);
	}
}