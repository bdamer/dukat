#include "stdafx.h"
#include "hexmap.h"

namespace dukat
{
	HexMap::HexMap(int width, int height) : width(width), height(height), cells(width * height), corners((2 * width + 1) * (2 * height + 1))
	{
		const float size = 1.0f;
		const float w = sqrtf(3.0f) * size;
		const float h = 2.0f * size;

		const int corners_width = 2 * width + 1;
		const int corners_height = 2 * height + 1;

		float pos_x, pos_y;
		for (auto y = 0; y < height; y++)
		{
			pos_x = ((y % 2 == 0) ? 0.0f : 0.5f * w);
			pos_y = 0.75f * h * (float)y;

			for (auto x = 0; x < width; x++)
			{
				auto& cell = cells[y * width + x];

				cell.border = (x == 0) || (x == width - 1) || (y == 0) || (y == height - 1);
				cell.ocean = x % 2;
		
				cell.pos.x = pos_x;
				cell.pos.y = pos_y;

				// populate corners
				auto cidx = 0;
				cell.corners[0] = &(corners[cidx]);
				cell.corners[1] = &(corners[cidx + 1]);
				cell.corners[2] = &(corners[cidx + 2]);
				cell.corners[3] = &(corners[cidx + corners_width + 2]);
				cell.corners[4] = &(corners[cidx + corners_width + 1]);
				cell.corners[5] = &(corners[cidx + corners_width]);

				// populate neighbors
				if (y > 0)
				{
					if (x > 0)
					{
						cell.neighbors[0] = &cells[(y - 1) * width + x];
					}
					if (x < width - 1)
					{
						cell.neighbors[1] = &cells[(y - 1) * width + x + 1];
					}
				}

				if (x < width - 1)
				{
					cell.neighbors[2] = &cells[y * width + x + 1];
				}

				if (y < height - 1)
				{
					if (x < width - 1)
					{
						cell.neighbors[3] = &cells[(y + 1) * width + x + 1];
					}
					if (x > 0)
					{
						cell.neighbors[4] = &cells[(y + 1) * width + x];
					}
				}

				if (x > 0)
				{
					cell.neighbors[5] = &cells[y * width + x - 1];
				}


				pos_x += w;
			}
		}

		// TODO: Populate corners - not tonight though!
		for (auto y = 0; y < corners_height; y++)
		{
			for (auto x = 0; x < corners_width; x++)
			{

			}
		}

	}

	HexMap::Cell& HexMap::get_cell(int x, int y)
	{
		return cells[y * width + x];
	}
}