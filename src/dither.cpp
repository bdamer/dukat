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

	void generate_bayer_matrix(std::vector<float>& m, int n)
	{
		std::vector<int> raw(m.size());
		generate_bayer_matrix(raw, n);
		for (auto i = 0u; i < m.size(); i++)
			m[i] = static_cast<float>(raw[i]) / static_cast<float>(m.size()) - 0.5f;
	}

	void dither_ordered(Surface& surface, int n, const std::array<Color, 2>& palette)
	{
		// Create normalized bayer matrix
		const auto dim = 1 << n;
		std::vector<float> m(dim * dim);
		generate_bayer_matrix(m, n);

		for (auto y = 0; y < surface.height(); y++)
		{
			for (auto x = 0; x < surface.width(); x++)
			{
				const auto error = m[(y % dim) * dim + (x % dim)];
				auto color = luminance(surface.get_color(x, y)) + error;
				if (color < 0.5f)
					surface.set_color(x, y, palette[0]);
				else
					surface.set_color(x, y, palette[1]);
			}
		}
	}
}
