#pragma once

#include <dukat/dukat.h>

namespace dukat
{
	class HexMap
	{
	public:
		struct Cell;

		struct Corner
		{
			Vector2 pos;
			float elevation;

			int x;
			int y;

			bool border;
			bool water;
			bool ocean;
			bool coast;

			std::array<Cell*, 3> cells;
			std::array<Corner*, 3> neighbors;

			Corner(void)
			{
				cells.fill(nullptr);
				neighbors.fill(nullptr);
			}
		};

		struct Cell
		{
			Vector2 pos;
			float elevation;

			int x;
			int y;

			bool border;
			bool water;
			bool ocean;
			bool coast;

			std::array<Corner*, 6> corners; // Corners of this cell in order (NW, N, NE, SE, S, SW)
			std::array<Cell*, 6> neighbors; // Neighboring cells in order (NW, N, NE, SE, S, SW)

			Cell(void) : pos({0.0f, 0.0f}), elevation(0.0f)
			{
				corners.fill(nullptr);
				neighbors.fill(nullptr);
			}
		};


	private:
		const int width;
		const int height;
		const float size;
		const float cell_width;
		const float cell_height;

		std::vector<Cell> cells;
		std::vector<Corner> corners;

	public:
		// Creates a new map with size <width> by <height>.
		// <size> defines the size of each map cell.
		HexMap(int width, int height, float size);

		// Retrieves a cell by axial coordinates.
		Cell* get_cell(int x, int y);
		// Returns cell by index - check if needed
		Cell* get_cell(int index) { return &(cells[index]); }

		int get_width(void) const { return width; }
		int get_height(void) const { return height; }
	
		void noise_pass(float frequency, int octaves, std::uint32_t seed);
	};
}