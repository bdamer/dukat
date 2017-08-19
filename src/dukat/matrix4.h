#pragma once

#include <GL/glew.h>
#include <cassert>

namespace dukat
{
	class Vector3;
	class Quaternion;

	// 4x4 matrix in column-major order.
	// 
	// m11 m12 m13 m14
	// m21 m22 m23 m24
	// m31 m32 m33 m34
	// m41 m42 m43 m44
	//
	// Memory layout:
	// m11, m21, m31, m41
	// m12, m22, m32, m42
	// m13, m23, m33, m43
	// m14, m24, m34, m44
	//
	// All methods assume a right-handed coordinate system:
	// - positive Z points towards the camera
	// - positive Y points up
	// - positive X points towards the left of the camera
	// - Rotations are performed clockwise looking from the origin in positive axis direction.
	class Matrix4
	{
	public:
		// Column-major ordered elements of the matrix.
		GLfloat m[16];

		Matrix4(void) { }
		~Matrix4(void) { }

		void identity(void);

		Matrix4 inverse(void) const;
		Matrix4 transpose(void) const;

		// Compute the determinant of the matrix
		float determinant(void) const;

		// Translates this matrix
		void translate(const Vector3& v);
		// Sets up a new translation matrix.
		void setup_translation(const Vector3& v);

		// Sets up a new scale matrix.
		void setup_scale(const Vector3& v);		
		// Sets up a new uniform scale matrix.
		void setup_scale(float k);

		// Sets up a new rotation matrix around an arbitrary axis.
		void setup_rotation(const Vector3& axis, float theta);
		// Sets up a new rotation matrix based on a quaternion.
		void setup_rotation(const Quaternion& q);
		// Sets up a new rotation matrix based on a set of rotation axis.
		void setup_rotation(const Vector3& left, const Vector3& up, const Vector3& dir);

		// Sets up a new orthographic projection matrix.
		void setup_orthographic(float left, float top, float right, float bottom, float near, float far);
		// Sets up a new perspective projection matrix.
		// fov - Vertical field of view
		// aspect - screen aspect ratio
		// near, far - clipping planes
		void setup_perspective(float fov_y, float aspect, float near, float far);
		// Sets up a new frustrum projection matrix.
		void setup_frustrum(float left, float right, float bottom, float top, float near, float far);

		// Extracts rotation axis from orthogonal rotation matrix.
		void extract_rotation_axis(Vector3& left, Vector3& up, Vector3& dir) const;
		// Extracts translation part from matrix.
		void extract_translation(Vector3& v) const;

		// Accessor for matrix elements.
		GLfloat& operator[] (const int index) { assert(index >= 0 && index < 16); return m[index]; }
		const GLfloat& operator[] (const int index) const { assert(index >= 0 && index < 16); return m[index]; }
	};

	// Operator * is used to transform a point, and it also concatenates matrics.
	// The order of multiplications from left to right is the same as
	// the order of transformations
	Vector3 operator *(const Vector3& p, const Matrix4& m);

	// Matrix concatenation. This makes using the vector class look like it
	// does with linear algebra notation on paper.
	// We also provide a *= operator, as per C convention.
	Matrix4 operator *(const Matrix4& a, const Matrix4& m);

	Matrix4 operator *(float s, const Matrix4& m);

	// Operator *= for conformance to C++ standards
	Vector3& operator *=(Vector3& p, const Matrix4& m);
	Matrix4& operator *=(Matrix4& a, const Matrix4& b);
	Matrix4& operator *=(Matrix4& m, const float s);
}
