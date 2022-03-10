#include "stdafx.h"
#include <dukat/rand.h>

namespace dukat
{
	namespace rand
	{
		static uint64_t s[2];

		static inline uint64_t rotl(const uint64_t x, int k) 
		{
			return (x << k) | (x >> (64 - k));
		}

		void seed(uint64_t seed)
		{
			s[0] = seed >> 32;
			s[1] = seed - (s[0] << 32);
		}

		uint64_t next(void)
		{
			const auto s0 = s[0];
			auto s1 = s[1];
			const auto result = rotl(s0 + s1, 17) + s0;

			s1 ^= s0;
			s[0] = rotl(s0, 49) ^ s1 ^ (s1 << 21); // a, b
			s[1] = rotl(s1, 28); // c

			return result;
		}

		void jump(void)
		{
			static const uint64_t JUMP[] = { 0x2bd7a6a6e99c2ddc, 0x0992ccaf6a6fca05 };

			uint64_t s0 = 0;
			uint64_t s1 = 0;
			for (auto i = 0; i < sizeof JUMP / sizeof * JUMP; i++)
			{
				for (auto b = 0; b < 64; b++) 
				{
					if (JUMP[i] & UINT64_C(1) << b) 
					{
						s0 ^= s[0];
						s1 ^= s[1];
					}
					next();
				}
			}

			s[0] = s0;
			s[1] = s1;
		}

		void long_jump(void)
		{
			static const uint64_t LONG_JUMP[] = { 0x360fd5f2cf8d5d99, 0x9c6e6877736c46e3 };

			uint64_t s0 = 0;
			uint64_t s1 = 0;
			for (auto i = 0; i < sizeof LONG_JUMP / sizeof * LONG_JUMP; i++)
			{
				for (auto b = 0; b < 64; b++) 
				{
					if (LONG_JUMP[i] & UINT64_C(1) << b) 
					{
						s0 ^= s[0];
						s1 ^= s[1];
					}
					next();
				}
			}

			s[0] = s0;
			s[1] = s1;
		}
	}

	float random(float min, float max)
	{
		if (min == max)
			return min;
		else
			return min + (max - min) * (static_cast<float>(static_cast<uint16_t>(rand::next())) / static_cast<float>(std::numeric_limits<uint16_t>::max()));
	}
	
	int random(int min, int max)
	{
		if (min == max) // protect against / 0
			return min;
		else
			return min + static_cast<uint32_t>(rand::next()) % (max - min);
	}
}