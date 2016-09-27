#pragma once

#include "vector2.h"

namespace dukat
{
	class Ray2
	{
	public:
		Vector2 origin;
		Vector2 dir;	
		Ray2() { };
		Ray2(const Vector2& o, const Vector2& d) : origin(o), dir(d) { };
		Vector2 point_at(float t) const { return origin + dir * t; }
	};
}