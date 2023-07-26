#pragma once

#include "vector2.h"

namespace dukat
{
	struct Ray2
	{
		Vector2 origin;
		Vector2 dir;
		Ray2(void) : origin({ 0, 0 }), dir({ 0, 0 }) { };
		Ray2(const Vector2& o, const Vector2& d) : origin(o), dir(d) { };
		Vector2 point_at(float t) const { return origin + dir * t; }
	};
}