#pragma once

#include <algorithm>

namespace dukat
{
	class Vector3
	{
	public:
		// Internally this is actually a 4-component vector, to simplify interation with OpenGL
		float x, y, z, w;

		// Constructors
		Vector3(void) : x(0), y(0), z(0), w(1) { }
		Vector3(const Vector3& a) : x(a.x), y(a.y), z(a.z) { }
		Vector3(float nx, float ny, float nz) : x(nx), y(ny), z(nz) { }

		// Vector operations
		Vector3& operator=(const Vector3& a) { x = a.x; y = a.y; z = a.z; return *this; }
		bool operator==(const Vector3& a) const { return x == a.x && y == a.y && z == a.z; }
		bool operator!=(const Vector3& a) const { return x != a.x || y != a.y || z != a.z; }
		Vector3 operator-() const { return Vector3(-x, -y, -z); }
		Vector3 operator+(const Vector3& a) const { return Vector3(x + a.x, y + a.y, z + a.z); }
		Vector3 operator-(const Vector3& a) const { return Vector3(x - a.x, y - a.y, z - a.z); }
		Vector3 operator*(float a) const { return Vector3(x * a, y * a, z * a); }
		Vector3 operator/(float a) const {	float one_over_a = 1.0f / a; return Vector3(x * one_over_a, y * one_over_a, z * one_over_a); }

		void zero() { x = y = z = 0.0f; }
		Vector3& normalize(void);
		// dot product
		float operator *(const Vector3& a) const { return x * a.x + y * a.y + z * a.z; }
		// returns the magnitude of the vector
		float mag() const { return sqrt(mag2()); }
		// returns the magnitude squared
		float mag2() const { return x * x + y * y + z * z; }

		Vector3 inverse(void) const { return Vector3(1.0f / x, 1.0f / y, 1.0f / z); }

		float max_el(void) const { return std::max(x, std::max(y, z)); }
		float min_el(void) const { return std::min(x, std::min(y, z)); }
		void sanitize(void);
		void randomize(float mag = 1.0f);

		friend std::ostream& operator<<(std::ostream& os, const Vector3& v);

		const static Vector3 origin;
		const static Vector3 unit_x;
		const static Vector3 unit_y;
		const static Vector3 unit_z;
	};

	// computes the cross product of two vectors
	inline Vector3 cross_product(const Vector3& a, const Vector3& b)
	{
		return Vector3(
			a.y*b.z - a.z*b.y,
			a.z*b.x - a.x*b.z,
			a.x*b.y - a.y*b.x);
	}

	// scalar on the left multiplication, for symmetry
	inline Vector3 operator*(float k, const Vector3 &v)
	{
		return Vector3(k*v.x, k*v.y, k*v.z);
	}

	// compute the distance between two points
	inline float distance(const Vector3& a, const Vector3& b)
	{
		float dx = a.x - b.x;
		float dy = a.y - b.y;
		float dz = a.z - b.z;
		return sqrt(dx*dx + dy*dy + dz*dz);
	}

	inline Vector3& operator+=(Vector3& a, const Vector3& b)
	{
		a = a + b;
		return a;
	}

	inline Vector3& operator-=(Vector3& a, const Vector3& b)
	{
		a = a - b;
		return a;
	}

	inline Vector3& operator*=(Vector3& a, float scalar)
	{
		a = a * scalar;
		return a;
	}

	inline Vector3& operator/=(Vector3& a, float scalar)
	{
		a = a / scalar;
		return a;
	}
}