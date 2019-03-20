#include "stdafx.h"
#include <dukat/bit.h>

namespace dukat
{
	uint32_t swape32(uint32_t value)
	{
		uint32_t result = 0;
		result |= (value & 0x000000FF) << 24;
		result |= (value & 0x0000FF00) << 8;
		result |= (value & 0x00FF0000) >> 8;
		result |= (value & 0xFF000000) >> 24;
		return result;
	}

	uint16_t swape16(uint16_t value)
	{
		uint16_t result = 0;
		result |= (value & 0x00FF) << 8;
		result |= (value & 0xFF00) >> 8;
		return result;
	}
}
