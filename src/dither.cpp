#include "stdafx.h"
#include <dukat/color.h>
#include <dukat/dither.h>
#include <dukat/mathutil.h>
#include <dukat/surface.h>
#include <array>

namespace dukat
{
	void generate_bayer_matrix(std::vector<int>& m, int n)
	{
		const auto dim = 1 << n;
		for (auto y = 0; y < dim; y++)
		{
			for (auto x = 0; x < dim; x++)
			{
				auto v = 0u;
				auto mask = n - 1;
				auto xc = x ^ y;
				const auto yc = y;
				for (auto bit = 0; bit < 2 * n; --mask)
				{
					v |= ((yc >> mask) & 1) << bit++;
					v |= ((xc >> mask) & 1) << bit++;
				}
				m[y * dim + x] = v;
			}
		}
	}

	void limit_color(float& color, float& error, float left = 0.0f, float right = 1.0f)
	{
		if (left > right)
			std::swap(left, right);

		const auto range = (right - left);
		const auto center = left + (range / 2.0f);
		color += error * range; // add scaled error term
		if (color < center)
		{
			error = color - left;
			color = left;
		}
		else
		{
			error = color - right;
			color = right;
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

	void dither_image_mono(const Surface& src, DitherAlgorithm<float, 1>& algorithm, const std::array<Color, 2>& palette, Surface& dest)
	{
		const auto width = src.width();
		const auto height = src.height();
		for (auto y = 0; y < height; y++)
		{
			algorithm.next_row();
			for (auto x = 0; x < width; x++)
			{
				auto color = src.get_color(x, y).r;
				auto error = algorithm.get_error(x, y);
				color += error[0];
				if (color < 0.5f)
				{
					error[0] = color;
					dest.set_color(x, y, palette[0]);
				}
				else
				{
					error[0] = color - 1.0f;
					dest.set_color(x, y, palette[1]);
				}
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

	// TODO: this isn't working correctly when using a color palette
	// I suspect we need to scale the error terms based on the avg difference between color values
	// in the palette.
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
