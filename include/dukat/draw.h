#pragma once

#include "surface.h"

namespace dukat
{
	// Draws a line from (x0,y0) to (x1,x0)
	void draw_line(Surface& surface, int x0, int y0, int x1, int y1, const Color& color);
	// Draws a circle with a given radius around (x,y)
	void draw_circle(Surface& surface, int x, int y, int radius, const Color& color);
	// Fills a circle with a given radius around (x,y)
	void fill_circle(Surface& surface, int x, int y, int radius, const Color& color);
	// Draws a rectangle with given dimensions. The top-left corner is at (x,y)
	void draw_rect(Surface& surface, int x, int y, int width, int height, const Color& color);
	// Fills surface with color.
	void fill(Surface& s, const Color& color);
	// Fills a rectangle with given dimensions. The top-left corner is at (x,y)
	void fill_rect(Surface& surface, int x, int y, int width, int height, const Color& color);
	void fill_rect(Surface& surface, const Rect& r, const Color& color);
	// Applies monochrome gradient to surface following the vertical axis.
	void gradient_v(Surface& surface, int from_r, int to_r, const Color& from_color, const Color& to_color);
	// Applies monochrome gradient to surface following the vertical axis.
	void gradient_h(Surface& surface, int from_r, int to_r, const Color& from_color, const Color& to_color);
	// Applies monochrome gradient to surface from x/y within radius (in pixels).
	void radial_gradient(Surface& surface, int x, int y, int from_r, int to_r, const Color& from_color, const Color& to_color);

	// Blends source surface on top of destination surface.
	void blend(const Surface& src, Surface& dest);
	void blend(const Surface& src, const Rect& src_rect, Surface& dest, const Rect& dest_rect);
	void blend(const Surface& src, int x, int y, Surface& dest);
	// Blends flipped source surface onto destination surface.
	void blend_flip_h(const Surface& src, const Rect& src_rect, Surface& dest, const Rect& dest_rect);
	void blend_flip_v(const Surface& src, const Rect& src_rect, Surface& dest, const Rect& dest_rect);

}