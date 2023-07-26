#pragma once

#include "vector2.h"

namespace dukat
{
	struct Ellipse
	{
		Vector2 origin;
		float a, b; // a is width / 2, b is height / 2
		Ellipse(void) : origin({ 0, 0 }), a(0.0f), b(0.0f) { }
		Ellipse(const Vector2& origin, float a, float b) : origin(origin), a(a), b(b) { }
		inline Vector2 point_at(float t) const { return origin + Vector2{ a * std::cos(t), b * std::sin(t) }; };
	};
}