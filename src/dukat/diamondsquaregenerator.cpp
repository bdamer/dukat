#include "stdafx.h"
#include "diamondsquaregenerator.h"
#include "mathutil.h"

namespace dukat
{
	void set(HeightMapLevel& level, int x, int y, float val)
	{
		if (x < 0)
		{
			x += level.size;
		}
		else if (x >= level.size)
		{
			x -= level.size;
		}
		if (y < 0)
		{
			y += level.size;
		}
		else if (y >= level.size)
		{
			y -= level.size;
		}
		level.data[y * level.size + x] = val;
	}

	float get(HeightMapLevel& level, int x, int y)
	{
		if (x < 0)
		{
			x += level.size;
		}
		else if (x >= level.size)
		{
			x -= level.size;
		}
		if (y < 0)
		{
			y += level.size;
		}
		else if (y >= level.size)
		{
			y -= level.size;
		}
		return level.data[y * level.size + x];
	}
	
	void DiamondSquareGenerator::generate(HeightMapLevel& level) const
	{
		srand(seed);

		// set corners
		set(level, 0, 0, 0.0f);
		set(level, level.size - 1, 0, 0.0f);
		set(level, 0, level.size - 1, 0.0f);
		set(level, level.size - 1, level.size - 1, 0.0f);
		divide(level, level.size);

		// Normalize data in [0..1] range
		float min_z = big_number;
		float max_z = -big_number;
		for (auto it = level.data.begin(); it != level.data.end(); ++it)
		{
			min_z = std::min(min_z, *it);
			max_z = std::max(max_z, *it);
		}
		const auto factor = 1.0f / (max_z - min_z);
		const auto half_size = 0.5f * (float)level.size;
		for (int y = 0; y < level.size; y++)
		{
			for (int x = 0; x < level.size; x++)
			{
				auto val = level.data[y * level.size + x];
				auto falloff = 1.0f;
				level.data[y * level.size + x] = (val - min_z) * factor * falloff;
			}
		}
	}

	void DiamondSquareGenerator::divide(HeightMapLevel& level, int size) const
	{
		auto half_size = size / 2;
		if (half_size < 1)
		{
			return;
		}
		
		auto scale = roughness * (float)size;
		for (auto y = half_size; y < level.size; y += size)
		{
			for (auto x = half_size; x < level.size; x += size)
			{
				square(level, x, y, half_size, randf(-scale, scale));
			}
		}

		for (auto y = 0; y <= level.size; y += half_size)
		{
			for (auto x = (y + half_size) % size; x <= level.size; x += size)
			{
				diamond(level, x, y, half_size, randf(-scale, scale));
			}
		}

		divide(level, half_size);
	}

	void DiamondSquareGenerator::square(HeightMapLevel& level, int x, int y, int size, float offset) const
	{
		auto avg = 0.25f * (get(level, x - size, y - size) + get(level, x + size, y - size) +
			get(level, x + size, y + size) + get(level, x - size, y + size));
		set(level, x, y, avg + offset);
	}

	void DiamondSquareGenerator::diamond(HeightMapLevel& level, int x, int y, int size, float offset) const
	{
		auto avg = 0.25f * (get(level, x, y - size) + get(level, x + size, y) +
			get(level, x, y + size) + get(level, x - size, y));
		set(level, x, y, avg + offset);
	}
}