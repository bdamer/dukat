#pragma once

namespace dukat
{
	struct Color
	{
		GLfloat r, g, b, a;

		Color operator+(const Color& c) const { return { r + c.r, g + c.g, b + c.b, a + c.a }; }
		Color operator-(const Color& c) const { return { r - c.r, g - c.g, b - c.b, a - c.a }; }
		Color operator*(float s) const { return { r * s, g * s, b * s, a * s }; }
		Color operator/(float s) const { auto one_over_s = 1.0f / s; return{ r * one_over_s, g * one_over_s, b * one_over_s, a * one_over_s }; }
		bool operator==(const Color& c) const { return (r == c.r) && (g == c.g) && (b == c.b) && (a == c.a); }
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

	inline Color& operator*=(Color& a, const Color& b)
	{
		a.r *= b.r;
		a.g *= b.g;
		a.b *= b.b;
		a.a *= b.a;
		return a;
	}

	inline Color& operator/=(Color& a, float scalar)
	{
		a = a / scalar;
		return a;
	}

	inline Color& operator/=(Color& a, const Color& b)
	{
		a.r /= b.r;
		a.g /= b.g;
		a.b /= b.b;
		a.a /= b.a;
		return a;
	}

	// Creates color from RGB hex representation.
	inline Color color_rgb(int rgb)
	{
		return Color { 
			static_cast<float>((rgb >> 16) & 0xff) / 255.0f,
			static_cast<float>((rgb >> 8) & 0xff) / 255.0f,
			static_cast<float>((rgb) & 0xff) / 255.0f,
			1.0f 
		};
	}

	// Creates color from RGBA hex representation.
	inline Color color_rgba(uint32_t rgba) 
	{
		return Color{
			static_cast<float>((rgba >> 24) & 0xff) / 255.0f,
			static_cast<float>((rgba >> 16) & 0xff) / 255.0f,
			static_cast<float>((rgba >> 8) & 0xff) / 255.0f,
			static_cast<float>((rgba) & 0xff) / 255.0f
		};
	}
}