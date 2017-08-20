#include "stdafx.h"
#include "quaternion.h"
#include "eulerangles.h"
#include "vector3.h"
#include "mathutil.h"
#include "matrix4.h"

namespace dukat
{
	const Quaternion Quaternion::unit(1.0f, 0.0f, 0.0f, 0.0f);

	Quaternion& Quaternion::set_to_rotate_x(float theta)
	{
		// compute the half angle
		const auto theta_over_2 = theta * 0.5f;
		// set the values
		w = std::cos(theta_over_2);
		x = std::sin(theta_over_2);
		y = 0.0f;
		z = 0.0f;
		return *this;
	}

	Quaternion& Quaternion::set_to_rotate_y(float theta)
	{
		// compute the half angle
		const auto theta_over_2 = theta * 0.5f;
		// set the values
		w = std::cos(theta_over_2);
		x = 0.0f;
		y = std::sin(theta_over_2);
		z = 0.0f;
		return *this;
	}

	Quaternion& Quaternion::set_to_rotate_z(float theta)
	{
		// compute the half angle
		const auto theta_over_2 = theta * 0.5f;
		// set the values
		w = std::cos(theta_over_2);
		x = 0.0f;
		y = 0.0f;
		z = std::sin(theta_over_2);
		return *this;
	}

	Quaternion& Quaternion::set_to_rotate_axis(const Vector3& axis, float theta)
	{
		// the axis of rotation must be normalized
		assert(std::abs(axis.mag() - 1.0f) < 0.01f);
		// compute the half angle and its sin
		const auto theta_over_2 = theta * 0.5f;
		const auto sin_theta_over_2 = std::sin(theta_over_2);
		// set the values
		w = std::cos(theta_over_2);
		x = axis.x * sin_theta_over_2;
		y = axis.y * sin_theta_over_2;
		z = axis.z * sin_theta_over_2;
		return *this;
	}

	Quaternion& Quaternion::from_vectors(const Vector3& a, const Vector3& b)
	{
		const auto m = std::sqrt(2.0f + 2.0f * (a * b));
		const auto v = (1.0f / m) * cross_product(a, b);
		w = 0.5f * m;
		x = v.x;
		y = v.y;
		z = v.z;
		return *this;
	}

	Quaternion& Quaternion::from_matrix(const Matrix4& m)
	{
		float tr = m.m[0] + m[5] + m[10];
		if (tr > 0.0f)
		{
			float s = 0.5f / sqrtf(tr + 1.0f);
			w = 0.25f / s;
			x = (m.m[9] - m.m[6]) * s;
			y = (m.m[2] - m.m[8]) * s;
			z = (m.m[4] - m.m[1]) * s;
		}
		else if ((m.m[0] > m.m[5]) && (m.m[0] > m.m[10]))
		{
			float s = 2.0f * sqrtf(1.0f + m.m[0] - m.m[5] - m.m[10]);
			w = (m.m[9] - m.m[6]) / s;
			x = 0.25f * s;
			y = (m.m[1] + m.m[4]) / s;
			z = (m.m[2] + m.m[8]) / s;
		}
		else if (m.m[5] > m.m[10])
		{
			float s = 2.0f * sqrtf(1.0f + m.m[5] - m.m[0] - m.m[10]);
			w = (m.m[2] - m.m[8]) / s;
			x = (m.m[1] + m.m[4]) / s;
			y = 0.25f * s;
			z = (m.m[6] + m.m[9]) / s;
		}
		else
		{
			float s = 2.0f * sqrtf(1.0f + m.m[10] - m.m[0] - m.m[5]);
			w = (m.m[4] - m.m[1]) / s;
			x = (m.m[2] + m.m[8]) / s;
			y = (m.m[6] + m.m[9]) / s;
			z = 0.25f * s;
		}
		return *this;
	}

	Quaternion& Quaternion::set_to_rotate_object_to_inertial(const EulerAngles& orientation)
	{
		// compute sine and cosine of the half angles
		float sp, sb, sh;
		float cp, cb, ch;
		sin_cos(sp, cp, orientation.pitch * 0.5f);
		sin_cos(sb, cb, orientation.bank * 0.5f);
		sin_cos(sh, ch, orientation.heading * 0.5f);
		// compute values
		w = ch*cp*cb + sh*sp*sb;
		x = ch*sp*cb + sh*cp*sb;
		y = -ch*sp*sb + sh*cp*cb;
		z = -sh*sp*cb + ch*cp*sb;
		return *this;
	}

	Quaternion& Quaternion::set_to_rotate_inertial_to_object(const EulerAngles& orientation)
	{
		// compute sine and cosine of the half angles
		float sp, sb, sh;
		float cp, cb, ch;
		sin_cos(sp, cp, orientation.pitch * 0.5f);
		sin_cos(sb, cb, orientation.bank * 0.5f);
		sin_cos(sh, ch, orientation.heading * 0.5f);
		// compute value
		w = ch*cp*cb + sh*sp*sb;
		x = -ch*sp*cb - sh*cp*sb;
		y = ch*sp*sb - sh*cb*cp;
		z = sh*sp*cb - ch*cp*sb;
		return *this;
	}

	Quaternion Quaternion::operator*(const Quaternion& a) const
	{
		Quaternion result;
		result.w = w*a.w - x*a.x - y*a.y - z*a.z;
		result.x = w*a.x + x*a.w + z*a.y - y*a.z;
		result.y = w*a.y + y*a.w + x*a.z - z*a.x;
		result.z = w*a.z + z*a.w + y*a.x - x*a.y;
		return result;
	}

	Quaternion& Quaternion::operator*=(const Quaternion& a)
	{
		// multiply and assign
		*this = *this * a;
		// return reference to l-value
		return *this;
	}

	void Quaternion::normalize()
	{
		// compute magnitude of the quaternion
		float mag = (float)sqrt(w*w + x*x + y*y +z*z);
		// check for bogus length to protect against divide by zero
		if (mag > 0.0f)
		{
			// normalize it
			float one_over_mag = 1.0f / mag;
			w *= one_over_mag;
			x *= one_over_mag;
			y *= one_over_mag;
			z *= one_over_mag;
		}
		else
		{
			// houston, we have a problem
			assert(false);
			// in a release build, just slam it to something
			identity();
		}
	}

	float Quaternion::rotation_angle() const
	{
		// compute the half angle. remember that w = cos(theta / 2)
		float theta_over_2 = safe_acos(w);
		// return the rotation angle
		return theta_over_2 * 2.0f;
	}

	Vector3 Quaternion::rotation_axis() const
	{
		// compute sin^2(theta/2). Remember that w = cos(theta/2),
		// and sin^2(x) + cos^2(x) = 1
		float sin_theta_over_2Sq = 1.0f - w*w;
		// protect against numerical imprecision
		if (sin_theta_over_2Sq <= 0.0f)
		{
			// identity quaternion, or numerical imprecision. just
			// return any valid vector, since it doesn't matter
			return Vector3(1.0f, 0.0f, 0.0f);
		}
	
		// compute 1 / sin(theta/2)
		float one_over_sin_theta_over_2 = 1.0f / sqrt(sin_theta_over_2Sq);
		// return axis of rotation
		return Vector3(
			x * one_over_sin_theta_over_2,
			y * one_over_sin_theta_over_2,
			z * one_over_sin_theta_over_2);
	}

	float dot_product(const Quaternion& a, const Quaternion& b)
	{
		return a.w*b.w + a.x*b.x + a.y*b.y + a.z*b.z;
	}

	Quaternion slerp(const Quaternion& q0, const Quaternion& q1, float t)
	{
		// check for out-of range parameter and return edge points if so
		if (t <= 0.0f)
			return q0;
		if (t >= 1.0f)
			return q1;
		// compute "cosine of angle between quaternions" using dot product
		float cos_omega = dot_product(q0, q1);
		// if negative dot, use -q1. Two quaternioons q and -q
		// represent the same rotation, but may produce
		// different slerp. We chose q or -q to rotate using
		// the acute angle.
		float q1w = q1.w;
		float q1x = q1.x;
		float q1y = q1.y;
		float q1z = q1.z;
		if (cos_omega < 0.0f)
		{
			q1w = -q1w;
			q1x = -q1x;
			q1y = -q1y;
			q1z = -q1z;
			cos_omega = -cos_omega;
		}

		// we should have two unit quaternions, so dot should be <= 1.0
		assert(cos_omega < 1.1f);

		// compute interpolation fraction, checking for quaternions
		// almost exactly the same
		float k0, k1;
		if (cos_omega < 0.9999f)
		{
			// very close - just use linear interpolation,
			// whjich will protect against a divide by zero
			k0 = 1.0f - t;
			k1 = t;
		}
		else
		{
			// compute the sin of the angle using the
			// trig identity sin^2(omega) + cos^2(omega) = 1
			float sin_omega = sqrt(1.0f - cos_omega*cos_omega);
			// compute the angle from its sin and cosine
			float omega = atan2(sin_omega, cos_omega);
			// compute inverse of denominator, so we only have
			// to divide once
			float one_over_sin_omega = 1.0f / sin_omega;
			// compute interpolation parameters
			k0 = sin((1.0f - t) * omega) * one_over_sin_omega;
			k1 = sin(t * omega) * one_over_sin_omega;
		}

		// interpolate
		Quaternion result;
		result.x = k0*q0.x + k1*q1x;
		result.y = k0*q0.y + k1*q1y;
		result.z = k0*q0.z + k1*q1z;
		result.w = k0*q0.w + k1*q1w;
		return result;
	}

	Quaternion Quaternion::conjugate(void) const
	{
		Quaternion result;
		// same rotation amount
		result.w = w;
		// opposite axis of rotation
		result.x = -x;
		result.y = -y;
		result.z = -z;
		return result;
	}

	Quaternion pow(const Quaternion& q, float exponent)
	{
		// check for the case of an identity quaternion.
		// this will protect against divide by zero
		if (fabs(q.w) > 0.9999f)
		{
			return q;
		}
		// extract the half angle alpha (alpha = theta / 2)
		float alpha = acos(q.w);
		// compute new alpha value
		float new_alpha = alpha * exponent;
		// compute new w value
		Quaternion result;
		result.w = cos(new_alpha);
		// compute new xyz values
		float mult = sin(new_alpha) / sin(alpha);
		result.x = q.x * mult;
		result.y = q.y * mult;
		result.z = q.z * mult;
		return result;
	}

	Vector3 operator*(const Quaternion & q, const Vector3 & v)
	{
		float xx = 2.0f * q.x; float yy = 2.0f * q.y; float zz = 2.0f * q.z;
		float xxx = xx * q.x; float yyy = yy * q.y; float zzz = zz * q.z;
		float yyx = yy * q.x; float zzx = zz * q.x; float zzy = zz * q.y;
		float xxw = xx * q.w; float yyw = yy * q.w; float zzw = zz * q.w;
		Vector3 result;
		result.x = (1.0f - (yyy + zzz)) * v.x + (yyx - zzw) * v.y + (zzx + yyw) * v.z;
		result.y = (yyx + zzw) * v.x + (1.0f - (xxx + zzz)) * v.y + (zzy - xxw) * v.z;
		result.z = (zzx - yyw) * v.x + (zzy + xxw) * v.y + (1.0f - (xxx + yyy)) * v.z;
		return result;
	}

	Vector3 & operator*=(Vector3 & v, const Quaternion & q)
	{
		float xx = 2.0f * q.x; float yy = 2.0f * q.y; float zz = 2.0f * q.z;
		float xxx = xx * q.x; float yyy = yy * q.y; float zzz = zz * q.z;
		float yyx = yy * q.x; float zzx = zz * q.x; float zzy = zz * q.y;
		float xxw = xx * q.w; float yyw = yy * q.w; float zzw = zz * q.w;
		float tx = (1.0f - (yyy + zzz)) * v.x + (yyx - zzw) * v.y + (zzx + yyw) * v.z;
		float ty = (yyx + zzw) * v.x + (1.0f - (xxx + zzz)) * v.y + (zzy - xxw) * v.z;
		float tz = (zzx - yyw) * v.x + (zzy + xxw) * v.y + (1.0f - (xxx + yyy)) * v.z;
		v.x = tx; v.y = ty; v.z = tz;
		return v;
	}
}
