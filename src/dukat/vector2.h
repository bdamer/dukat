#pragma once

#include <math.h>
#include <iostream>

namespace dukat
{
	class Vector2
	{
	public:
		float x, y;

		// Constructors
		Vector2(void) : x(0), y(0) { }
		Vector2(int x, int y) : x((float)x), y((float)y) { }
		Vector2(float x, float y) : x(x), y(y) { }
		Vector2(const Vector2& v) : x(v.x), y(v.y) { }
		
		// Vector operations
		bool operator==(const Vector2& a) const { return x == a.x && y == a.y; }
		bool operator!=(const Vector2& a) const { return x != a.x || y != a.y; }
		Vector2 operator+(const Vector2& a) const { return Vector2(x + a.x, y + a.y); }
		Vector2 operator-(const Vector2& a) const { return Vector2(x - a.x, y - a.y); }
		Vector2 operator*(float a) const { return Vector2(x * a, y * a); }
		Vector2 operator/(float a) const {	float oneOverA = 1.0f / a; return Vector2(x * oneOverA, y * oneOverA); }
		Vector2 operator-(void) const { return Vector2(-x, -y); }

		// Dot product
		float operator*(const Vector2& a) const { return x * a.x + y * a.y; } 

		float mag2(void) const { return x * x + y * y; }
		float mag(void) const { return sqrt(mag2()); }
		void normalize(void) { float oneOverMag = 1 / mag(); x *= oneOverMag; y *= oneOverMag; } 

		bool in_range(const Vector2& min, const Vector2& max) const { return (min.x <= x) && (x <= max.x) && (min.y <= y) && (y <= max.y); }
		static Vector2 random(const Vector2& min, const Vector2& max);

		// Rotation
		Vector2 rotate(float theta) const;
		// Angle between two vectors. This vector and v must be normalized.
		float angle_between(const Vector2& v) const;
		// Angle of this vector.
		float angle(void) const;

		friend std::ostream& operator<<(std::ostream& os, const Vector2& v);

		const static Vector2 origin;
		const static Vector2 unit_x;
		const static Vector2 unit_y;
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
}