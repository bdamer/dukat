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
			float moisture;
			bool border;
			bool water;
			bool ocean;
			bool coast;
			int river; // 0 if no river or volume of water in river
			int watershed_size;
			Corner* downslope; // points to adjacent corner most downhill
			Corner* watershed; // points to coastal corner or null

			std::array<Cell*, 3> cells;
			std::array<Corner*, 3> adjacent;

			int x, y; // TODO: used for debugging

			Corner(void) : pos({ 0.0f, 0.0f }), elevation(0.0f), moisture(0.0f), 
				border(false), water(false), ocean(false), coast(false), river(0), watershed_size(0),
				downslope(nullptr), watershed(nullptr)
			{
				cells.fill(nullptr);
				adjacent.fill(nullptr);
			}
		};

		struct Cell
		{
			Vector2 pos;
			float elevation;
			float moisture;
			bool border;
			bool water;
			bool ocean;
			bool coast;
			std::array<Corner*, 6> corners; // Corners of this cell in order (NW, N, NE, SE, S, SW)
			std::array<Cell*, 6> neighbors; // Neighboring cells in order (NW, N, NE, SE, S, SW)

			int x, y; // TODO: used for debugging

			Cell(void) : pos({0.0f, 0.0f}), elevation(0.0f), moisture(0.0f), border(false), water(false), ocean(false), coast(false)
			{
				corners.fill(nullptr);
				neighbors.fill(nullptr);
			}
		};

		struct River
		{
			std::vector<Corner*> corners;
		};

	private:
		// map width and height in cells
		const int width;
		const int height;
		// cell dimensions
		const float size;
		const float cell_width;
		const float cell_height;
		// cell buffer
		std::vector<Cell> cells;

		void create_rivers(int num_rivers);
		void create_lakes(void);

	public:
		// TODO: make private

		// width and height of corner buffer
		const int corners_width;
		const int corners_height;
		std::vector<Corner> corners;

		std::vector<River> rivers;

		// Creates a new map with size <width> by <height>.
		// <size> defines the size of each map cell.
		HexMap(int width, int height, float size);

		// Retrieves a cell by axial coordinates.
		Cell* get_cell(int x, int y);
		// Returns cell by index - check if needed
		Cell* get_cell(int index) { return &(cells[index]); }

		int get_width(void) const { return width; }
		int get_height(void) const { return height; }

		void reset(void);

		void elevation_pass(float frequency, int octaves, std::uint32_t seed);
		void moisture_pass(void);
	};
}