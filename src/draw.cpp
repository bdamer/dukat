#include "stdafx.h"
#include <dukat/draw.h>

namespace dukat
{
	void draw_line(Surface& surface, int x0, int y0, int x1, int y1, uint32_t color)
	{
		// check if slope is steep
		const auto steep = abs(y1 - y0) > abs(x1 - x0);
		if (steep)
		{
			std::swap(x0, y0);
			std::swap(x1, y1);
		}
		// check if line is drawn from right-to-left
		if (x0 > x1)
		{
			std::swap(x0, x1);
			std::swap(y0, y1);
		}

		const auto dx = x1 - x0;
		const auto dy = abs(y1 - y0);
		auto error = dx / 2;
		auto y = y0;
		const auto y_step = (y0 < y1) ? 1 : -1;
		for (auto x = x0; x < x1; x++)
		{
			if (x < 0 || x >= surface.width())
				continue;
			if (y < 0 || y >= surface.height())
				continue;

			if (steep)
				surface(y, x) = color;
			else
				surface(x, y) = color;
			error = error - dy;
			if (error < 0)
			{
				y = y + y_step;
				error = error + dx;
			}
		}
	}

	void draw_circle(Surface& surface, int x, int y, int radius, uint32_t color)
	{
		auto px = radius;
		auto py = 0;
		auto radiusError = 1 - px;
		while (px >= py)
		{
			surface(px + x, py + y) = color;
			surface(py + x, px + y) = color;
			surface(-px + x, py + y) = color;
			surface(-py + x, px + y) = color;
			surface(-px + x, -py + y) = color;
			surface(-py + x, -px + y) = color;
			surface(px + x, -py + y) = color;
			surface(py + x, -px + y) = color;
			py++;

			if (radiusError < 0)
			{
				radiusError += 2 * py + 1;
			}
			else
			{
				px--;
				radiusError += 2 * (py - px + 1);
			}
		}
	}

	void draw_rect(Surface& surface, int x, int y, int width, int height, uint32_t color)
	{
		draw_line(surface, x, y, x + width, y, color);
		draw_line(surface, x + width, y, x + width, y + height, color);
		draw_line(surface, x + width, y + height, x, y + height, color);
		draw_line(surface, x, y + height, x, y, color);
	}

	void fill_circle(Surface& surface, int x, int y, int radius, uint32_t color)
	{
		auto px = radius;
		auto py = 0;
		auto radiusError = 1 - px;
		while (px >= py)
		{
			draw_line(surface, x - px, py + y, x + px, py + y, color);
			draw_line(surface, x - py, px + y, x + py, px + y, color);
			draw_line(surface, x - px, y - py, x + px, y - py, color);
			draw_line(surface, x - py, y - px, x + py, y - px, color);
			y++;

			if (radiusError < 0)
			{
				radiusError += 2 * py + 1;
			}
			else
			{
				px--;
				radiusError += 2 * (py - px + 1);
			}
		}
	}

	void fill_rect(Surface& surface, int x, int y, int width, int height, uint32_t color)
	{
		const SDL_Rect rect = { x, y, width, height };
		SDL_FillRect(surface.get_surface(), &rect, color);
	}

	void fill_rect(Surface& surface, const Rect& r, uint32_t color)
	{
		fill_rect(surface, r.x, r.y, r.w, r.h, color);
	}
}