#include "stdafx.h"
#include "heightmap.h"
#include "heightmapgenerator.h"
#include "log.h"
#include "mathutil.h"
#include "rect.h"
#include "surface.h"
#include "ray3.h"
#include <png.h>

namespace dukat
{
    void HeightMap::generate_levels(void)
    {
        // For now, done on the CPU - investigate performance gains for doing this on the GPU
        for (auto i = 1; i < num_levels; i++)
        {
            levels.push_back({ i, level_size >> i });

            auto cur_stride = levels[i].size;
            auto prev_stride = levels[i - 1].size;

            for (auto y = 0; y < cur_stride; y++)
            {
                for (auto x = 0; x < cur_stride; x++)
                {
                    auto prev_y = y * 2;
                    auto prev_x = x * 2;
                    auto i0 = prev_y * prev_stride + prev_x;
                    auto i1 = prev_y * prev_stride + prev_x + 1;
                    auto i2 = (prev_y + 1) * prev_stride + prev_x;
                    auto i3 = (prev_y + 1) * prev_stride + prev_x + 1;

                    // Perform bilinear filtering of previous level to generate this level 
                    auto z0 = levels[i - 1].data[i0];
                    auto z1 = levels[i - 1].data[i1];
                    auto z2 = levels[i - 1].data[i2];
                    auto z3 = levels[i - 1].data[i3];
					levels[i].data[y * cur_stride + x] = (z0 + z1 + z2 + z3) / 4.0f;
                }                
            }
        }
    }

	void HeightMap::load(const std::string& filename)
	{
        // reset level structure
        if (!levels.empty())
        {
            levels.clear();
        }

		png_image img;
		memset(&img, 0, sizeof(img));
		img.version = PNG_IMAGE_VERSION;

		if (!png_image_begin_read_from_file(&img, filename.c_str()))
		{
			throw std::runtime_error("Could not load heightmap.");
		}
		if (img.format != PNG_FORMAT_LINEAR_Y)
		{
			throw std::runtime_error("Unsupported format - expected 16 bit grayscale.");
		}

		// allocate buffer to hold image data
		const auto num_pixels = PNG_IMAGE_SIZE(img) / sizeof(uint16_t);
		std::vector<uint16_t> buffer(num_pixels);

		if (!png_image_finish_read(&img, nullptr, buffer.data(), 0, nullptr))
		{
			throw std::runtime_error("Failed to read height map data.");
		}

		const auto max_val = std::numeric_limits<uint16_t>::max();
		const auto factor = 1.0f / (float)max_val;

        // Create level 0
        assert(img.width == img.height);
        level_size = img.width;
        levels.push_back({ 0, level_size });

		// normalize data and store at level 0
		for (auto i = 0u; i < num_pixels; i++)
		{
			auto z = (float)buffer[i];
			levels[0].data[i] = factor * z;
		}

		png_image_free(&img);
	
		generate_levels();
	}

	void HeightMap::save(const std::string& filename) const
	{
		png_image img;
		memset(&img, 0, sizeof(img));
		img.version = PNG_IMAGE_VERSION;
		img.format = PNG_FORMAT_LINEAR_Y; // 16-bit grayscale
		img.width = level_size;
		img.height = level_size;

		const auto num_pixels = PNG_IMAGE_SIZE(img) / sizeof(uint16_t);
		std::vector<uint16_t> buffer(num_pixels);
		const auto max_val = std::numeric_limits<uint16_t>::max();

		for (auto i = 0u; i < num_pixels; i++)
		{
			buffer[i] = (uint16_t)(levels[0].data[i] * (float)max_val);
		}

		// TODO: error handling
		png_image_write_to_file(&img, filename.c_str(), 0, buffer.data(), 0, nullptr);
	}

	void HeightMap::allocate(int level_size)
	{
		if (!levels.empty())
		{
			levels.clear();
		}

		this->level_size = level_size;
		levels.push_back({ 0, level_size });

		generate_levels();
	}

	void HeightMap::generate(int level_size, const HeightMapGenerator& gen)
	{
		if (!levels.empty())
		{
			levels.clear();
		}

		this->level_size = level_size;
		levels.push_back({ 0, level_size });

		// initialize 1st level
		gen.generate(levels[0]);

		generate_levels();
	}

    void HeightMap::get_data(int level, const Rect& rect, std::vector<GLfloat>& buffer) const
    {
		const auto stride = levels[level].size;
		const auto last_row = std::min(rect.y + rect.h, stride);
		const auto last_col = std::min(rect.x + rect.w, stride);

		auto dst = buffer.begin();
		auto y = rect.y;

		// Outside of bounds < 0
		if (y < 0)
		{
			auto count = abs(y) * rect.w;
			std::fill(dst, dst + count, 0.0f);
			dst += count;
			y = 0;
		}

        for ( ; y < last_row; y++)
        {
			auto x = rect.x;
			// outside of bounds < 0
			if (x < 0)
			{
				auto count = abs(x);
				std::fill(dst, dst + count, 0.0f);
				dst += count;
				x = 0;
			}

			if (x < last_col)
			{
				auto src = levels[level].data.begin() + y * stride;
				std::copy(src + x, src + last_col, dst);
				dst += (last_col - x);
			}

			// outside of bounds > level_size
			if (last_col < rect.x + rect.w)
			{
				auto count = (rect.x + rect.w) - last_col;
				std::fill(dst, dst + count, 0.0f);
				dst += count;
			}
        }

		// Outside of bounds > level_size
		if (last_row < rect.y + rect.h)
		{
			std::fill(dst, buffer.end(), 0.0f);
		}
    }

	float HeightMap::get_elevation(int x, int y, int level) const
	{
		assert(level < num_levels);
		const auto stride = levels[level].size;
		if (x < 0 || x >= stride || y < 0 || y >= stride)
		{
			return 0.0f;
		}
		else
		{
			return levels[level].data[y * stride + x];
		}
	}

	float HeightMap::sample(int level, float x, float y) const
	{
		assert(level < num_levels);
		const auto stride = levels[level].size;
		// sample 4 surrounding elevations
		auto min_x = (int)std::floor(x);
		auto min_y = (int)std::floor(y);
		auto max_x = min_x + 1;
		auto max_y = min_y + 1;
		auto z0 = get_elevation(min_x, min_y, level);
		auto z1 = get_elevation(max_x, min_y, level);
		auto z2 = get_elevation(min_x, max_y, level);
		auto z3 = get_elevation(max_x, max_y, level);
		// compute distance to each corner
		auto dx0 = x - (float)min_x;
		auto dy0 = y - (float)min_y;
		auto dx1 = (float)max_x - x;
		auto dy1 = (float)max_y - y;
		return (z0 * dx1 * dy1) + (z1 * dx0 * dy1) + (z2 * dx1 * dy0) + (z3 * dx0 * dy0);
	}

	float HeightMap::intersect_ray(const Ray3& ray, float min_t, float max_t) const
	{
		const auto step_size = std::sqrt(2.0f);
		Vector3 cur;
		float t = min_t;
		while (t < max_t)
		{
			cur = ray.origin + ray.dir * t;
			auto elevation = scale_factor * get_elevation((int)std::round(cur.x), (int)std::round(cur.z), 0);
			if (elevation > cur.y)
			{
				return t;
			}
			t += step_size;
		}

		return no_intersection;
	}
}