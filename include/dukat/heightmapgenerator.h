#pragma once

#include <vector>
#include "heightmap.h"

namespace dukat
{
	class HeightMapGenerator
	{
	public:
		HeightMapGenerator(void) { }
		virtual ~HeightMapGenerator(void) { }

		virtual void generate(HeightMap::Level& level) const = 0;
	};
}