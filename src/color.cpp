#include "stdafx.h"
#include <dukat/color.h>

namespace dukat
{
	Color rgb_to_hsl(const Color& rgb)
	{
		// R = H
		// G = S
		// B = L
		Color hsl;

		const auto max_v = std::max(std::max(rgb.r, rgb.g), rgb.b);
		const auto min_v = std::min(std::min(rgb.r, rgb.g), rgb.b);

		hsl.r = hsl.g = hsl.b = (max_v + min_v) / 2.0f;
		hsl.a = rgb.a;

		const auto d = max_v - min_v;
		if (d == 0.0f)
		{
			hsl.r = hsl.g = 0.0f; // achromatic
		}
		else
		{
			hsl.g = (hsl.b > 0.5f) ? d / (2.0f - max_v - min_v) : d / (max_v + min_v);
			if (max_v == rgb.r)
				hsl.r = (rgb.g - rgb.b) / d + (rgb.g < rgb.b ? 6.f : 0.f);
			else if (max_v == rgb.g)
				hsl.r = (rgb.b - rgb.r) / d + 2.f;
			else if (max_v == rgb.b)
				hsl.r = (rgb.r - rgb.g) / d + 4.f;
			hsl.r /= 6.f;
		}

		return hsl;
	}

	float hue_to_rgb(float p, float q, float t) {

		if (t < 0.f)
			t += 1.f;
		if (t > 1.f)
			t -= 1.f;
		if (t < 1. / 6.f)
			return p + (q - p) * 6.f * t;
		if (t < 1.f / 2.f)
			return q;
		if (t < 2.f / 3.f)
			return p + (q - p) * (2.f / 3.f - t) * 6.f;
		return p;
	}

	Color hsl_to_rgb(const Color& hsl)
	{
		Color rgb;
		rgb.a = hsl.a;

		if (0.0f == hsl.g)
		{
			rgb.r = rgb.g = rgb.b = hsl.b / 255.0f; // achromatic
		}
		else
		{
			const auto q = hsl.b < 0.5f ? hsl.b * (1.f + hsl.g) : hsl.b + hsl.g - hsl.b * hsl.g;
			const auto p = 2.f * hsl.b - q;
			rgb.r = hue_to_rgb(p, q, hsl.r + 1.f / 3.f);
			rgb.g = hue_to_rgb(p, q, hsl.r);
			rgb.b = hue_to_rgb(p, q, hsl.r - 1.f / 3.f);
		}

		return rgb;
	}
}