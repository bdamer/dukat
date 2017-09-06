#pragma once

namespace dukat
{
	struct Rect
	{
		int x, y;
		int w, h;

		friend std::ostream& operator<<(std::ostream& os, const Rect& r)
		{
			os << r.x << "," << r.y << "x" << r.w << "," << r.h;
			return os;
		}
	};
}