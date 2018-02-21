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
		std::vector<Cell> cells;
		std::vector<Corner> corners;

	public:
		HexMap(int width, int height);

		// Retrieves a cell by axial coordinates.
		Cell& get_cell(int x, int y);

		int get_width(void) const { return width; }
		int get_height(void) const { return height; }
	};
}