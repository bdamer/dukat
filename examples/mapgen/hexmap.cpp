#include "stdafx.h"
#include "hexmap.h"
#include "thirdparty/PerlinNoise.hpp"

namespace dukat
{
	// recompute cell elevation by averaging corners
	template<typename T, typename U>
	void assign_cell_elevation(std::vector<T>& cells)
	{
		for (auto& cell : cells)
		{
			auto sum = std::accumulate(cell.corners.begin(), cell.corners.end(), 0.0f,
				[&](float total, U* corner) { return total + corner->elevation; });
			cell.elevation = sum / cell.corners.size();		
		}
	}

	// Computes downslopes for a list of cell corners.
	template<typename T>
	void calculate_downslopes(std::vector<T>& corners)
	{
		for (auto& q : corners)
		{
			auto r = &q;
			for (auto& s : q.adjacent)
			{
				if (s != nullptr && s->elevation <= r->elevation)
				{
					r = s;
				}
			}
			q.downslope = r;
		}
	}

	template<typename T>
	void calculate_watersheds(std::vector<T>& corners)
	{
		// Initially the watershed pointer points downslope one step.      
		for (auto& q : corners)
		{
			q.watershed = &q;
			if (!q.ocean && !q.coast)
			{
				q.watershed = q.downslope;
			}
		}
		// Follow the downslope pointers to the coast. Limit to 100
		// iterations although most of the time with numPoints==2000 it
		// only takes 20 iterations because most points are not far from
		// a coast.  
		for (int i = 0; i < 100; i++)
		{
			bool changed = false;
			for (auto& q : corners)
			{
				if (!q.ocean && !q.coast && !q.watershed->coast)
				{
					auto r = q.downslope->watershed;
					if (!r->ocean)
					{
						q.watershed = r;
						changed = true;
					}
				}
			}
			if (!changed)
				break;
		}
		// How big is each watershed?
		for (auto& q : corners)
		{
			auto r = q.watershed;
			r->watershed_size = 1 + r->watershed_size;
		}
	}

	// Compute moisture levels for river corners.
	template<typename T>
	void assign_corner_moisture(std::vector<T>& corners)
	{
		std::queue<T*> queue;
		for (auto& q : corners)
		{
			if ((q.water || q.river > 0) && !q.ocean)
			{
				q.moisture = q.river > 0 ? std::min(2.0f, 0.2f * static_cast<float>(q.river)) : 1.0f;
				queue.push(&q);
			}
			else
			{
				q.moisture = 0.0f;
			}
		}
		while (!queue.empty())
		{
			auto q = queue.front();
			queue.pop();
			for (auto r : q->adjacent)
			{
				auto new_moisture = q->moisture * 0.7f;
				if (r != nullptr && new_moisture > r->moisture)
				{
					r->moisture = new_moisture;
					queue.push(r);
				}
			}
		}
	}

	template<typename T>
	void normalize_moisture(std::vector<T>& corners)
	{
		std::vector<T*> locations;
		for (auto& c : corners)
		{
			locations.push_back(&c);
		}
		std::sort(locations.begin(), locations.end(), [](T* a, T *b) -> bool {
			return a->moisture < b->moisture;
		});
		for (auto i = 0u; i < locations.size(); i++)
		{
			locations[i]->moisture = (float)i / (float)(locations.size() - 1);
		}
	}

	template<typename T>
	void MapGraph::assign_cell_moisture(std::vector<T>& cells)
	{
		for (auto& p : cells)
		{
			auto sum = 0.0f;
			for (auto q : p.corners)
			{
				if (q->moisture > 1.0f)
					q->moisture = 1.0f;
				sum += q->moisture;
			}
			p.moisture = sum / (float)p.corners.size();
		}
	}

	HexMap::HexMap(int width, int height, float size) : width(width), height(height), size(size),
		corners_width(2 * width + 2), corners_height(height + 1), cell_width(sqrtf(3.0f) * size), cell_height(2.0f * size)
	{
		reset();
	}

	void HexMap::reset(void)
	{
		cells.clear();
		corners.clear();
		rivers.clear();

		cells.resize(width * height);
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
				cell.pos.x = pos_x;
				cell.pos.y = pos_y;
				cell.border = (x == 0) || (x == width - 1) || (y == 0) || (y == height - 1);

				cell.x = x; cell.y = y;

				// populate corners
				auto cidx = (y * corners_width) + 2 * x + y % 2;
				cell.corners[0] = &(corners[cidx + 1]); // N
				cell.corners[0]->cells[0] = &cell;
				cell.corners[1] = &(corners[cidx]); // NW
				cell.corners[1]->cells[1] = &cell;
				cell.corners[2] = &(corners[cidx + corners_width]); // SW
				cell.corners[2]->cells[2] = &cell;
				cell.corners[3] = &(corners[cidx + corners_width + 1]); // S
				cell.corners[3]->cells[0] = &cell;
				cell.corners[4] = &(corners[cidx + corners_width + 2]); // SE
				cell.corners[4]->cells[1] = &cell;
				cell.corners[5] = &(corners[cidx + 2]); // NE
				cell.corners[5]->cells[2] = &cell;

				// compute axial coordinates
				auto ax = x - y / 2;
				auto ay = y;

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
				corner.border = (x == 0) || (x == corners_width - 1) || (y == 0) || (y == corners_height - 1);
				corner.coast = corner.border;
				corner.x = x; corner.y = y;

				// skip ghost corners in first
				if (idx == (corners_width - 1))
					continue;
				// skip ghost corner in last row
				if (y == corners_height - 1) 
				{
					if ((height % 2 == 0) && (x == 0)) // even number of cell rows -> ghost on 1st
						continue;
					if ((height % 2 == 1) && (x == corners_width - 1)) // odd number of cell rows -> ghost on last
						continue;
				}

				// TODO: check for ghost corners on first and last row - should remove connections to them
				if (((x + y) % 2) == 0) // y-shape
				{
					corner.pos.y = (0.25f + static_cast<float>(y) * 0.75f) * cell_height;
					corner.adjacent[0] = x > 0 ? &(corners[idx - 1]) : nullptr; // NW
					corner.adjacent[1] = x < corners_width - 1 ? &(corners[idx + 1]) : nullptr; // NE
					corner.adjacent[2] = y < corners_height - 1 ? &(corners[idx + corners_width]) : nullptr; // S
				}
				else // lambda shape
				{
					corner.pos.y = static_cast<float>(y) * 0.75f * cell_height;
					corner.adjacent[0] = x > 0 ? &(corners[idx - 1]) : nullptr; // SW
					corner.adjacent[1] = x < corners_width - 1 ? &(corners[idx + 1]) : nullptr; // SE
					corner.adjacent[2] = y > 0 ? &(corners[idx - corners_width]) : nullptr; // N
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

	void HexMap::elevation_pass(float frequency, int octaves, std::uint32_t seed)
	{
		clamp(frequency, 0.1f, 64.0f);
		clamp(octaves, 1, 16);

		const siv::PerlinNoise perlin(seed);
		const auto fx = (width * cell_width) / frequency;
		const auto fy = (height * cell_height) / frequency;

		for (auto& corner : corners)
		{
			auto x = corner.pos.x / fx;
			auto y = corner.pos.y / fy;
			corner.elevation = static_cast<float>(perlin.octaveNoise0_1(x, y, octaves));
			//corner.water = (corner.elevation < 0.2f || corner.elevation > 0.8f); // assign water to top elevation to simulate snowpack
		}

		assign_cell_elevation<HexMap::Cell, HexMap::Corner>(cells);
		calculate_downslopes(corners);
		calculate_watersheds(corners);
	}

	void HexMap::moisture_pass(void)
	{
		const float lake_threshold = 0.3f; // 0 to 1, fraction of water corners for water polygon
		for (auto& cell : cells)
		{
			auto num_water = 0;
			for (auto q : cell.corners)
			{
				if (q->border)
				{
					cell.border = true;
					cell.coast = true; // for now, assume outside of the map is coast to have water flow that way
				}
				if (q->water)
				{
					num_water++;
				}
			}
			cell.water = (cell.ocean || num_water >= cell.corners.size() * lake_threshold);
		}

		create_rivers(32);
		create_lakes();

		assign_corner_moisture(corners);

		// TODO: form lakes

		//normalize_moisture(corners);
		assign_cell_moisture(cells);
	}

	void HexMap::create_rivers(int num_rivers)
	{
		int i = 0;
		while (rivers.size() < num_rivers && i++ < 5000)
		{
			auto q = &corners[std::rand() % (corners.size() - 1)];
			if (q->coast || q->ocean || q->elevation < 0.8f)
				continue;

			River r;
			while (!q->coast)
			{
				r.corners.push_back(q);
				if (q == q->downslope)
					break;
				q->river = q->river + 1;
				q->downslope->river = q->downslope->river + 1;
				q = q->downslope;
			}

			if (r.corners.size() < 5)
				continue; // cull very short rivers

			logger << "Adding river starting at elev: " << r.corners[0]->elevation << " of length: " << r.corners.size() << std::endl;
			rivers.push_back(r);
		}
	}

	void HexMap::create_lakes(void)
	{
		for (auto& r : rivers)
		{
			auto last = r.corners.back();
			logger << "Last river: " << last->x << "," << last->y << std::endl;
			// TODO: nullptr
			//last->cells[0]->water = true;
			//last->cells[1]->water = true;
			//last->cells[2]->water = true;
		}
	}
}