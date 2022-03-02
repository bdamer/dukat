#pragma once

#include "surface.h"

namespace dukat
{
	// Draws a line from (x0,y0) to (x1,x0)
	void draw_line(Surface& surface, int x0, int y0, int x1, int y1, uint32_t color);
	// Draws a circle with a given radius around (x,y)
	void draw_circle(Surface& surface, int x, int y, int radius, uint32_t color);
	// Fills a circle with a given radius around (x,y)
	void fill_circle(Surface& surface, int x, int y, int radius, uint32_t color);
	// Draws a rectangle with given dimensions. The top-left corner is at (x,y)
	void draw_rect(Surface& surface, int x, int y, int width, int height, uint32_t color);
	// Fills a rectangle with given dimensions. The top-left corner is at (x,y)
	void fill_rect(Surface& surface, int x, int y, int width, int height, uint32_t color);
	void fill_rect(Surface& surface, const Rect& r, uint32_t color);

}