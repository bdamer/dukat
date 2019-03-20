#pragma once

// Low-level bit manipulations
namespace dukat
{
	// Checks a value against a bit mask
	inline bool check_flag(int val, int mask)
	{
		return ((val & mask) == mask);
	}

	// Swap endianess
	uint32_t swape32(uint32_t value);
	uint16_t swape16(uint16_t value);
}