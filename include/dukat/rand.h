#pragma once

#include <cstdlib>

namespace dukat
{
	namespace rand
	{
		// Seeds the random generator.
		void seed(uint64_t seed);
		// Returns next random value.
		uint64_t next(void);
		// Jump function for generator. Equivalent to 2^64 calls to next.
		void jump(void);
		// Long-jump function for generator. Equivalent to 2^96 calls to next.
		void long_jump(void);
	}

	// Utility methods for returning a random value within a range.
	float random(float min, float max);
	int random(int min, int max);

}