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

		template<typename T>
		struct Generator 
		{
			typedef T result_type;
			static T min(void) { return std::numeric_limits<T>::min(); }
			static T max(void) { return std::numeric_limits<T>::max(); }
			T operator()() { return static_cast<T>(next()); }
		};

		template<typename Iterator, typename RNG>
		void shuffle(Iterator first, Iterator last, RNG&& g)
		{
			if (first == last)
				return;

			const auto range = last - first;
			for (auto i = range - 1; i > 0; i--)
			{
				auto j = g.operator()() % i;
				std::iter_swap(first + i, first + j);
			}	
		}
	}

	// Utility methods for returning a random value within a range.
	float random(float min, float max);
	int random(int min, int max);

}