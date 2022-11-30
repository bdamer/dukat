#include "stdafx.h"
#include <dukat/color.h>
#include <dukat/dither.h>
#include <dukat/mathutil.h>
#include <dukat/surface.h>
#include <array>

namespace dukat
{
	void generate_bayer_matrix(std::vector<int>& m, size_t stride)
	{
		for (auto y = 0u; y < stride; y++)
		{
			for (auto x = 0u; x < stride; x++)
			{
				auto v = 0u;
				auto mask = stride - 1;
				auto xc = x ^ y;
				const auto yc = y;
				for (auto bit = 0u; bit < 2 * stride; --mask)
				{
					v |= ((yc >> mask) & 1) << bit++;
					v |= ((xc >> mask) & 1) << bit++;
				}
				m[y * stride + x] = v / (stride * stride);
			}
		}
	}

	void limit_color(float& color, float& error)
	{
		color += error;
		if (color < 0.5f)
		{
			error = color;
			color = 0.f;
		}
		else
		{
			error = color - 1.f;
			color = 1.f;
		}
	}

	void dither_image(const Surface& src, DitherAlgorithm<float, 3>& algorithm, Surface& dest)
	{
		const auto width = src.width();
		const auto height = src.height();
		for (auto y = 0; y < height; y++)
		{
			algorithm.next_row();
			for (auto x = 0; x < width; x++)
			{
				auto color = src.get_color(x, y);
				auto error = algorithm.get_error(x, y);
				limit_color(color.r, error[0]);
				limit_color(color.g, error[1]);
				limit_color(color.b, error[2]);
				dest.set_color(x, y, color);
				algorithm.distribute_error(x, y, error);
			}
		}
	}

	int find_closest_color(const Color& color, const std::vector<Color>& palette)
	{
		auto idx = 0;
		float best = big_number;
		for (auto i = 0; i < static_cast<int>(palette.size()); i++)
		{
			const auto& entry = palette[i];
			const auto rd = color.r - entry.r;
			const auto gd = color.g - entry.g;
			const auto bd = color.b - entry.b;
			const auto d = std::sqrt(rd * rd + gd * gd + bd * bd);

			if (d < best)
			{
				best = d;
				idx = i;
			}
		}
		return idx;
	}

	void dither_image(const Surface& src, DitherAlgorithm<float, 3>& algorithm, const std::vector<Color>& palette, Surface& dest)
	{
		// convert palette entries from RGB to HSL
		std::vector<Color> tmp_palette;
		for (const auto& it : palette)
			tmp_palette.push_back(rgb_to_hsl(it));

		const auto width = src.width();
		const auto height = src.height();
		for (auto y = 0; y < height; y++)
		{
			algorithm.next_row();
			for (auto x = 0; x < width; x++)
			{
				auto rgb = src.get_color(x, y);

				// apply error correction
				auto error = algorithm.get_error(x, y);
				rgb.r += error[0];
				rgb.g += error[1];
				rgb.b += error[2];

				auto hsl = rgb_to_hsl(rgb);
				auto closest_idx = find_closest_color(hsl, tmp_palette);
				const auto& closest = palette[closest_idx];
				error = {
					rgb.r - closest.r,
					rgb.g - closest.g,
					rgb.b - closest.b
				};

				dest.set_color(x, y, closest);
				algorithm.distribute_error(x, y, error);
			}
		}
	}
}
