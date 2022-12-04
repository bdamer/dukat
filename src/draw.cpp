#include "stdafx.h"
#include <dukat/draw.h>
#include <dukat/mathutil.h>
#include <dukat/vector2.h>
#include <array>

namespace dukat
{
	void draw_line(Surface& surface, int x0, int y0, int x1, int y1, const Color& color)
	{
		const auto c = surface.raw_color(color);

		// check if slope is steep
		const auto steep = std::abs(y1 - y0) > std::abs(x1 - x0);
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
		const auto dy = std::abs(y1 - y0);
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
				surface.set_pixel(y, x, c);
			else
				surface.set_pixel(x, y, c);
			error = error - dy;
			if (error < 0)
			{
				y = y + y_step;
				error = error + dx;
			}
		}
	}

	void draw_circle(Surface& surface, int x, int y, int radius, const Color& color)
	{
		const auto c = surface.raw_color(color);

		auto px = radius;
		auto py = 0;
		auto radiusError = 1 - px;
		while (px >= py)
		{
			surface.set_pixel(px + x, py + y, c);
			surface.set_pixel(py + x, px + y, c);
			surface.set_pixel(-px + x, py + y, c);
			surface.set_pixel(-py + x, px + y, c);
			surface.set_pixel(-px + x, -py + y, c);
			surface.set_pixel(-py + x, -px + y, c);
			surface.set_pixel(px + x, -py + y, c);
			surface.set_pixel(py + x, -px + y, c);
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

	void draw_rect(Surface& surface, int x, int y, int width, int height, const Color& color)
	{
		draw_line(surface, x, y, x + width, y, color);
		draw_line(surface, x + width, y, x + width, y + height, color);
		draw_line(surface, x + width, y + height, x, y + height, color);
		draw_line(surface, x, y + height, x, y, color);
	}

	void fill_circle(Surface& surface, int x, int y, int radius, const Color& color)
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

	void fill(Surface& surface, const Color& color)
	{
		SDL_FillRect(surface.get_surface(), nullptr, surface.raw_color(color));
	}

	void fill_rect(Surface& surface, int x, int y, int width, int height, const Color& color)
	{
		const SDL_Rect rect = { x, y, width, height };
		SDL_FillRect(surface.get_surface(), &rect, surface.raw_color(color));
	}

	void fill_rect(Surface& surface, const Rect& r, const Color& color)
	{
		SDL_FillRect(surface.get_surface(), (SDL_Rect*)&r, surface.raw_color(color));
	}

	void gradient_v(Surface& surface, int from_r, int to_r, const Color& from_color, const Color& to_color)
	{
		// per-component steps - this assumes range is 1. for each component
		std::array<float, 3> delta = {
			(to_color.r - from_color.r) / static_cast<float>(to_r - from_r),
			(to_color.g - from_color.g) / static_cast<float>(to_r - from_r),
			(to_color.b - from_color.b) / static_cast<float>(to_r - from_r)
		};

		auto cur = from_color;
		for (auto y = 0; y < surface.height(); y++)
		{
			fill_rect(surface, Rect{ 0, y, surface.width(), 1 }, cur);
			if (y >= from_r && y < to_r)
			{
				cur.r += delta[0];
				cur.g += delta[1];
				cur.b += delta[2];
			}
		}
	}

	void gradient_h(Surface& surface, int from_r, int to_r, const Color& from_color, const Color& to_color)
	{
		// per-component steps - this assumes range is 1. for each component
		std::array<float, 3> delta = {
			(to_color.r - from_color.r) / static_cast<float>(to_r - from_r),
			(to_color.g - from_color.g) / static_cast<float>(to_r - from_r),
			(to_color.b - from_color.b) / static_cast<float>(to_r - from_r)
		};

		auto cur = from_color;
		for (auto x = 0; x < surface.width(); x++)
		{
			fill_rect(surface, Rect{ x, 0, 1, surface.height() }, cur);
			if (x >= from_r && x < to_r)
			{
				cur.r += delta[0];
				cur.g += delta[1];
				cur.b += delta[2];
			}
		}
	}

	void radial_gradient(Surface& surface, int x, int y, int from_r, int to_r, const Color& from_color, const Color& to_color)
	{
		const auto dr = static_cast<float>(to_r) - static_cast<float>(from_r);
		const Vector2 center{ x, y };
		for (auto y = 0; y < surface.height(); y++)
		{
			for (auto x = 0; x < surface.width(); x++)
			{
				const auto dist = (Vector2{ x, y } - center).mag();
				auto g = (dist - static_cast<float>(from_r)) / dr; // normalize
				clamp(g, 0.0f, 1.0f);
				// interpolate 
				const auto color = Color{
					lerp(from_color.r, to_color.r, g),
					lerp(from_color.g, to_color.g, g),
					lerp(from_color.b, to_color.b, g),
					lerp(from_color.a, to_color.a, g)
				};
				surface.set_color(x, y, color);
			}
		}
	}

	void blend(const Surface& src, Surface& dest)
	{
		SDL_BlitSurface(src.get_surface(), nullptr, dest.get_surface(), nullptr);
	}

	void blend(const Surface& src, const Rect& src_rect, Surface& dest, const Rect& dest_rect)
	{
		SDL_BlitSurface(src.get_surface(), (SDL_Rect*)&src_rect, dest.get_surface(), (SDL_Rect*)&dest_rect);
	}

	void blend(const Surface& src, int x, int y, Surface& dest)
	{
		SDL_Rect r{ x, y, src.width(), src.height() };
		SDL_BlitSurface(src.get_surface(), nullptr, dest.get_surface(), &r);
	}

	void blend_flip_h(const Surface& src, const Rect& src_rect, Surface& dest, const Rect& dest_rect)
	{
		auto surface = src.get_surface();
		Surface tmp_surface(SDL_CreateRGBSurface(0, src.width(), src.height(), surface->format->BitsPerPixel,
			surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask));
		blend(src, src_rect, tmp_surface, src_rect);
		tmp_surface.flip_horizontal();
		SDL_BlitSurface(tmp_surface.get_surface(), nullptr, dest.get_surface(), (SDL_Rect*)&dest_rect);
	}

	void blend_flip_v(const Surface& src, const Rect& src_rect, Surface& dest, const Rect& dest_rect)
	{
		auto surface = src.get_surface();
		Surface tmp_surface(SDL_CreateRGBSurface(0, src.width(), src.height(), surface->format->BitsPerPixel,
			surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask));
		blend(src, src_rect, tmp_surface, src_rect);
		tmp_surface.flip_vertical();
		SDL_BlitSurface(tmp_surface.get_surface(), nullptr, dest.get_surface(), (SDL_Rect*)&dest_rect);
	}
}