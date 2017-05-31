#pragma once

#include "heightmapgenerator.h"

namespace dukat
{
	class DiamondSquareGenerator : public HeightMapGenerator
	{
	private:
		int seed;
		float roughness;
		float min_val, max_val; // range of output values [0..1]

		void divide(HeightMap::Level& level, int size) const;
		void square(HeightMap::Level& level, int x, int y, int size, float offset) const;
		void diamond(HeightMap::Level& level, int x, int y, int size, float offset) const;

	public:
		DiamondSquareGenerator(int seed = 0) : seed(seed), min_val(0.0f), max_val(1.0f) { }
		~DiamondSquareGenerator(void) { }

		void generate(HeightMap::Level& level) const;

		void set_roughness(float roughness) { this->roughness = roughness; }
		void set_range(float min_val, float max_val) { this->min_val = min_val; this->max_val = max_val; }
	};
}