#include "stdafx.h"
#include "hexmap.h"

namespace dukat
{
	HexMap::HexMap(int width, int height, float size) : width(width), height(height), size(size),
		cell_width(sqrtf(3.0f) * size), cell_height(2.0f * size), cells(width * height)
	{
		const int corners_width = 2 * width + 2;
		const int corners_height = height + 1;
		corners.resize(corners_width * corners_height);

		float pos_x, pos_y;
		// Populate cells and create links to corners and adjacent cells
		for (auto y = 0; y < height; y++)
		{
			pos_x = ((y % 2 == 0) ? 0.5f : 1.0f) * cell_width;
			pos_y = (0.5f + 0.75f * static_cast<float>(y)) * cell_height;

			for (auto x = 0; x < width; x++)
			{
				auto idx = y * width + x;
				auto& cell = cells[idx];

				cell.x = x;
				cell.y = y;

				cell.border = (x == 0) || (x == width - 1) || (y == 0) || (y == height - 1);
				cell.ocean = x % 2;
		
				cell.pos.x = pos_x;
				cell.pos.y = pos_y;

				// populate corners
				auto cidx = (y * corners_width) + 2 * x + y % 2;
				cell.corners[0] = &(corners[cidx + 1]); // N
				cell.corners[1] = &(corners[cidx]); // NW
				cell.corners[2] = &(corners[cidx + corners_width]); // SW
				cell.corners[3] = &(corners[cidx + corners_width + 1]); // S
				cell.corners[4] = &(corners[cidx + corners_width + 2]); // SE
				cell.corners[5] = &(corners[cidx + 2]); // NE

				// compute axial coordinates
				int ax = x - y / 2;
				int ay = y;

				// populate neighbors
				cell.neighbors[0] = get_cell(ax, ay - 1); // NW
				cell.neighbors[1] = get_cell(ax - 1, ay); // W
				cell.neighbors[2] = get_cell(ax - 1, ay + 1); // SW
				cell.neighbors[3] = get_cell(ax, ay + 1); // SE
				cell.neighbors[4] = get_cell(ax + 1, ay); // E
				cell.neighbors[5] = get_cell(ax + 1, ay - 1); // NE

				pos_x += cell_width;
			}
		}

		// Populate corners and create links to cells & adjacent corners
		for (auto y = 0; y < corners_height; y++)
		{
			for (auto x = 0; x < corners_width; x++)
			{
				auto idx = y * corners_width + x;
				auto& corner = corners[idx];
				corner.pos.x = static_cast<float>(x) * 0.5f * cell_width;

				// TODO: remove - used for debugging only
				corner.x = x;
				corner.y = y;

				// TODO: check for ghost corners on first and last row - should remove connections to them
				if (((x + y) % 2) == 0)
				{
					corner.pos.y = (0.25f + static_cast<float>(y) * 0.75f) * cell_height;
					corner.neighbors[0] = x > 0 ? &(corners[idx - 1]) : nullptr; // NW
					corner.neighbors[1] = x < corners_width - 1 ? &(corners[idx + 1]) : nullptr; // NE
					corner.neighbors[2] = y < corners_height - 1 ? &(corners[idx + corners_width]) : nullptr; // S
					corner.cells[0] = ((x > 0) && (x < corners_width - 1) && (y > 0)) ? &(cells[(y - 1) * width + (x - 1) / 2]) : nullptr;  // N
					corner.cells[1] = ((x < corners_width - 1) && (y < corners_height - 1)) ? &(cells[y * width + (x - 1) / 2]) : nullptr;  // SE
					corner.cells[2] = ((x > 0) && (y < corners_height - 1)) ? &(cells[y * width + (x - 2) / 2]) : nullptr;  // SW
				}
				else
				{
					corner.pos.y = static_cast<float>(y) * 0.75f * cell_height;
					corner.neighbors[0] = x > 0 ? &(corners[idx - 1]) : nullptr; // SW
					corner.neighbors[1] = x < corners_width - 1 ? &(corners[idx + 1]) : nullptr; // SE
					corner.neighbors[2] = y > 0 ? &(corners[idx - corners_width]) : nullptr; // N
					corner.cells[0] = ((x > 0) && (x < corners_width - 1) && (y < corners_height - 1)) ? &(cells[y * width + (x - 1) / 2]) : nullptr;  // S
					corner.cells[1] = ((x > 0) && (y > 0)) ? &(cells[(y - 1) * width + (x - 1) / 2]) : nullptr;  // NW
					corner.cells[2] = ((x < corners_width - 1) && (y > 0)) ? &(cells[(y - 1) * width + x / 2]) : nullptr;  // NE
				}
			}
		}
	}

	HexMap::Cell* HexMap::get_cell(int x, int y)
	{
		auto offset = x + y / 2;
		if (y < 0 || y >= height || offset < 0 || offset >= width)
			return nullptr;
		else
			return &(cells[y * width + offset]);
	}
}