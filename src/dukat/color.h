#pragma once

#include <GL/glew.h>

namespace dukat
{
	struct Color
	{
		GLfloat r, g, b, a;

		Color operator+(const Color& c) const { return { r + c.r, g + c.g, b + c.b, a + c.a }; }
		Color operator-(const Color& c) const { return { r - c.r, g - c.g, b - c.b, a - c.a }; }
		Color operator*(float s) const { return { r * s, g * s, b * s, a * s }; }
		Color operator/(float s) const { float one_over_s = 1.0f / s; return{ r * one_over_s, g * one_over_s, b * one_over_s, a * one_over_s }; }
	};

	inline Color& operator+=(Color& a, const Color& b)
	{
		a = a + b;
		return a;
	}

	inline Color& operator-=(Color& a, const Color& b)
	{
		a = a - b;
		return a;
	}

	inline Color& operator*=(Color& a, float scalar)
	{
		a = a * scalar;
		return a;
	}

	inline Color& operator/=(Color& a, float scalar)
	{
		a = a / scalar;
		return a;
	}
}