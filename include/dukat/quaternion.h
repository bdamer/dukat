#pragma once

namespace dukat
{
	class Vector3;
	class Matrix4;
	class EulerAngles;

	// Implements a quaternion for the purpose of representing an angular 
	// displacement (orientation) in 3D.
	class Quaternion
	{
	public:
		// The four values of the quaternion. Normally, it will not
		// be necessary to manipulate these directly. However,
		// we leave them public, since prohibiting direct access
		// makes some operations, such as file I/O, unnecessarily
		// complicated.
		float w, x, y, z;

		Quaternion(void) { w = 1.0f, x = y = z = 0.0f; }
		Quaternion(float w, float x, float y, float z) : w(w), x(x), y(y), z(z) { }
		Quaternion(const Vector3& axis, float theta) { set_to_rotate_axis(axis, theta); }

		// set to identity
		void identity() { w = 1.0f, x = y = z = 0.0f; }
		bool is_identity(void) const { return (x == 0.0f && y == 0.0f && z == 0.0f && w == 1.0f); }
		
		// setup the quaternion to a specific rotation
		Quaternion& set_to_rotate_x(float theta);
		Quaternion& set_to_rotate_y(float theta);
		Quaternion& set_to_rotate_z(float theta);
		Quaternion& set_to_rotate_axis(const Vector3& axis, float theta);

		// setup quaternion based on rotation between two vectors
		Quaternion& from_vectors(const Vector3& a, const Vector3& b);
		// setup quaternion based on rotation matrix.
		Quaternion& from_matrix(const Matrix4& m);

		// setup to perform object<->inertial rotations,
		// given orientation in Euler angle format.
		Quaternion& set_to_rotate_object_to_inertial(const EulerAngles& orientation);
		Quaternion& set_to_rotate_inertial_to_object(const EulerAngles& orientation);

		// cross product
		Quaternion operator*(const Quaternion& a) const;

		// multiplication with assignment, as per c++ convention
		Quaternion& operator*=(const Quaternion &a);

		bool operator==(const Quaternion& q) const { return (x == q.x && y == q.y && z == q.z && w == q.w); }
		bool operator!=(const Quaternion& q) const { return (x != q.x || y != q.y || z != q.z || w == q.w); }

		// normalize the quaternion
		void normalize(void);
		// quaternion conjugation
		Quaternion conjugate(void) const;

		// Extract and return the rotation angle and axis.
		float rotation_angle(void) const;
		Vector3 rotation_axis(void) const;

		// a global "identity" quaternion constant
		const static Quaternion unit;
	};

	// quaternion dot product
	extern float dot_product(const Quaternion& a, const Quaternion& b);

	// spehrical linear interpolation
	extern Quaternion slerp(const Quaternion& p, const Quaternion& q, float t);

	// quaternion exponentiation
	extern Quaternion pow(const Quaternion& q, float exponent);

	// Rotation of vector around quaternion
	extern Vector3 operator*(const Quaternion& q, const Vector3& v);
	extern Vector3& operator*=(Vector3& v, const Quaternion& q);
}
