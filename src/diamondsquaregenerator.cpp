#include "stdafx.h"
#include <dukat/diamondsquaregenerator.h>
#include <dukat/rand.h>

namespace dukat
{
	void set(HeightMap::Level& level, int x, int y, float val)
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
		level[y * level.size + x] = val;
	}

	float get(HeightMap::Level& level, int x, int y)
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
		return level[y * level.size + x];
	}
	
	void DiamondSquareGenerator::generate(HeightMap::Level& level) const
	{
		srand(seed);

		// set corners
		set(level, 0, 0, 0.0f);
		set(level, level.size - 1, 0, 0.0f);
		set(level, 0, level.size - 1, 0.0f);
		set(level, level.size - 1, level.size - 1, 0.0f);
		divide(level, level.size);

		// Normalize data in [min_val..max_val] range
		auto min_z = level.min();
		auto max_z = level.max();
		const auto factor = 1.0f / (max_z - min_z);
		const auto half_size = 0.5f * (float)level.size;
		for (int y = 0; y < level.size; y++)
		{
			for (int x = 0; x < level.size; x++)
			{
				auto val = level[y * level.size + x];
				auto falloff = 1.0f;
				auto normalized = (val - min_z) * factor * falloff;
				level[y * level.size + x] = normalized * (max_val - min_val) + min_val;
			}
		}
	}

	void DiamondSquareGenerator::divide(HeightMap::Level& level, int size) const
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
				square(level, x, y, half_size, random(-scale, scale));
			}
		}

		for (auto y = 0; y <= level.size; y += half_size)
		{
			for (auto x = (y + half_size) % size; x <= level.size; x += size)
			{
				diamond(level, x, y, half_size, random(-scale, scale));
			}
		}

		divide(level, half_size);
	}

	void DiamondSquareGenerator::square(HeightMap::Level& level, int x, int y, int size, float offset) const
	{
		auto avg = 0.25f * (get(level, x - size, y - size) + get(level, x + size, y - size) +
			get(level, x + size, y + size) + get(level, x - size, y + size));
		set(level, x, y, avg + offset);
	}

	void DiamondSquareGenerator::diamond(HeightMap::Level& level, int x, int y, int size, float offset) const
	{
		auto avg = 0.25f * (get(level, x, y - size) + get(level, x + size, y) +
			get(level, x, y + size) + get(level, x - size, y));
		set(level, x, y, avg + offset);
	}
}