#pragma once

namespace dukat
{
	struct Rect
	{
		int x, y;
		int w, h;

		Rect(void) : x(0), y(0), w(0), h(0) { }
		Rect(int x, int y, int w, int h) : x(x), y(y), h(h), w(w) { }
	};
}