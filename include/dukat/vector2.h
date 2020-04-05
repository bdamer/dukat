#pragma once

#include <cmath>
#include <iostream>
#include <dukat/mathutil.h>

namespace dukat
{
	class Vector2
	{
	public:
		float x, y;

		// Constructors
		Vector2(void) : x(0), y(0) { }
		Vector2(int x, int y) : x(static_cast<float>(x)), y(static_cast<float>(y)) { }
		Vector2(float x, float y) : x(x), y(y) { }
		Vector2(const Vector2& v) : x(v.x), y(v.y) { }
		
		// Vector operations
		bool operator==(const Vector2& a) const { return x == a.x && y == a.y; }
		bool operator!=(const Vector2& a) const { return x != a.x || y != a.y; }
		Vector2 operator+(const Vector2& a) const { return Vector2{ x + a.x, y + a.y }; }
		Vector2 operator-(const Vector2 & a) const { return Vector2{ x - a.x, y - a.y }; }
		Vector2 operator*(float a) const { return Vector2{ x * a, y * a}; }
		Vector2 operator/(float a) const { const auto one_over_a = 1.0f / a; return Vector2{ x * one_over_a, y * one_over_a }; }
		Vector2 operator-(void) const { return Vector2{ -x, -y }; }
		// Dot product
		float operator*(const Vector2& a) const { return x * a.x + y * a.y; } 

		// Magnitude
		float mag(void) const { return std::sqrt(mag2()); }
		float mag2(void) const { return x * x + y * y; }
		void set_mag(float magnitude) { const auto k = magnitude / mag(); x *= k; y *= k; }
		void zero(void) { x = y = 0.0f;  }
		void limit(float max_mag) { const auto m = mag(); const auto k = std::min(m, max_mag) / m; x *= k; y *= k; }
		void normalize(void) { const auto one_over_mag = 1.0f / mag(); x *= one_over_mag; y *= one_over_mag; }
		// Faster, less accurate normalization method.
		Vector2& normalize_fast(void);

		// Rotation - positive angles will rotate in CW direction.
		Vector2 rotate(float theta) const;
		// Angle of this vector (relative to [0,1]).
		float angle(void) const;

		friend std::ostream& operator<<(std::ostream& os, const Vector2& v);

		static const Vector2 origin;
		static const Vector2 unit_x;
		static const Vector2 unit_y;
	};

	inline Vector2& operator+=(Vector2& a, const Vector2& b)
	{
		a = a + b;
		return a;
	}

	inline Vector2& operator-=(Vector2& a, const Vector2& b)
	{
		a = a - b;
		return a;
	}

	inline Vector2& operator*=(Vector2& a, float b)
	{
		a = a * b;
		return a;
	}

	inline Vector2& operator/=(Vector2& a, float b)
	{
		a = a / b;
		return a;
	}

	// Returns linear interpolation between p and q.
	inline Vector2 lerp(const Vector2& p, const Vector2& q, float t)
	{
		return p + (q - p) * t; 
	}

	inline Vector2 round(const Vector2& v) 
	{ 
		return Vector2{ std::round(v.x), std::round(v.y) }; 
	}

	inline Vector2 abs(const Vector2& v) 
	{ 
		return Vector2{ std::abs(v.x), std::abs(v.y) }; 
	}
	
	inline Vector2 random(const Vector2& min, const Vector2& max)
	{
		return Vector2{ randf(min.x, max.x), randf(min.y, max.y) };
	}

	inline bool in_range(const Vector2& v, const Vector2& min, const Vector2& max)
	{
		return (min.x <= v.x) && (v.x <= max.x) && (min.y <= v.y) && (v.y <= max.y);
	}

	// Angle between two vectors. Both vectors must be normalized.
	float compute_angle(const Vector2& a, const Vector2& b);

	// Populates list with uniformly distributed coordinates.
	void generate_distribution(std::vector<Vector2>& data, const Vector2& min_v = Vector2{ 0, 0 }, const Vector2& max_v = Vector2{ 1, 1 });
}