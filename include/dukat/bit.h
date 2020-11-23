#pragma once

// Low-level bit manipulations
namespace dukat
{
	// Checks a value against a bit mask
	inline bool check_flag(int val, int mask)
	{
		return ((val & mask) == mask);
	}

	// Sets or clears a specific bit mask.
	inline void set_flag(int& val, int mask, bool set)
	{
		if (set) 
			val |= mask; 
		else 
			val &= ~mask;
	}

	// Swap endianess
	uint32_t swape32(uint32_t value);
	uint16_t swape16(uint16_t value);

	// Methods to flip buffer contents
	template<typename T>
	inline void flip_vertical(T* from, T* to, size_t pitch)
	{
		while (from < to)
		{
			to = to - pitch; // move <to> to start of row
			for (auto i = 0u; i < pitch; i++)
			{
				T tmp = *from;
				*from++ = *to;
				*to++ = tmp;
			}
			to = to - pitch;
		}
	}

	template<typename T>
	inline void flip_horizontal(T* from, T* to, size_t pitch)
	{
		const auto rows = (to - from) / pitch;
		for (auto i = 0u; i < rows; i++)
		{
			T* left = from + i * pitch;
			T* right = left + pitch - 1;
			while (left < right)
			{
				T tmp = *left;
				*left++ = *right;
				*right-- = tmp;
			}

		}
	}
}