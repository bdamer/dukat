#pragma once

#include "heightmapgenerator.h"

namespace dukat
{
	class DiamondSquareGenerator : public HeightMapGenerator
	{
	private:
		int seed;
		float roughness;

		void divide(HeightMapLevel& level, int size) const;
		void square(HeightMapLevel& level, int x, int y, int size, float offset) const;
		void diamond(HeightMapLevel& level, int x, int y, int size, float offset) const;

	public:
		DiamondSquareGenerator(int seed = 0) : seed(seed) { }
		~DiamondSquareGenerator(void) { }

		void generate(HeightMapLevel& level) const;

		void set_roughness(float roughness) { this->roughness = roughness; }
	};
}