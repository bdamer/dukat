#pragma once

namespace dukat
{
	class Vector2;

	class Matrix2
	{
	public:
		GLfloat m[4];

		Matrix2(void) : m() { }
		~Matrix2(void) { }

		// Makes this an identity matrix.
		void identity(void);

		// Sets up rotation matrix for a given CW angle.
		void setup_rotation(float angle);

		// Accessor for matrix elements.
		GLfloat& operator[] (const int index) { assert(index >= 0 && index < 4); return m[index]; }
		const GLfloat& operator[] (const int index) const { assert(index >= 0 && index < 4); return m[index]; }
	};

	// Operator * is used to transform a point, and it also concatenates matrics.
	// The order of multiplications from left to right is the same as
	// the order of transformations
	Vector2 operator *(const Vector2& p, const Matrix2& m);
}
