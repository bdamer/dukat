#include "stdafx.h"
#include "matrix4.h"
#include "mathutil.h"
#include "vector3.h"
#include "quaternion.h"

namespace dukat
{
	void Matrix4::identity(void)
	{
		m[ 0] = 1.0f; m[ 1] = 0.0f; m[ 2] = 0.0f; m[ 3] = 0.0f;
		m[ 4] = 0.0f; m[ 5] = 1.0f; m[ 6] = 0.0f; m[ 7] = 0.0f;
		m[ 8] = 0.0f; m[ 9] = 0.0f; m[10] = 1.0f; m[11] = 0.0f;
		m[12] = 0.0f; m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;
	}

	Matrix4 Matrix4::inverse(void) const
	{
		Matrix4 r = *this;

		// compute the determinant
		float det = determinant();
		// if we're singular, then the determinant is zero and there's no inverse
		// assert(fabs(det) > 0.000001f);
		if (det == 0.0)
			return r;
		// compute one over the determinant, so we divide once and
		// can *multiply* per element
		float one_over_det = 1.0f / det;

		r[0] = (m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] +
			    m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10]) * one_over_det;
		r[1] = (-m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - 
				m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10]) * one_over_det;
		r[2] = (m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] +
				m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6]) * one_over_det;
		r[3] = (-m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] -
				m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6]) * one_over_det;
		r[4] = (-m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] -
				m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10]) * one_over_det;
		r[5] = (m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] +
				m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10]) * one_over_det;
		r[6] = (-m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] -
				m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6]) * one_over_det;
		r[7] = (m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] +
				m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6]) * one_over_det;
		r[8] = (m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] +
				m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9]) * one_over_det;
		r[9] = (-m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] -
				m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9]) * one_over_det;
		r[10] = (m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] +
				 m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5]) * one_over_det;
		r[11] = (-m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] -
				 m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5]) * one_over_det;
		r[12] = (-m[4] * m[9] * m[14] +	m[4] * m[10] * m[13] + m[8] * m[5] * m[14] -
				 m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9]) * one_over_det;
		r[13] = (m[0] * m[9] * m[14] - m[0] * m[10] * m[13] -	m[8] * m[1] * m[14] +
				 m[8] * m[2] * m[13] +	m[12] * m[1] * m[10] -	m[12] * m[2] * m[9]) * one_over_det;
		r[14] = (-m[0] * m[5] * m[14] +	m[0] * m[6] * m[13] +	m[4] * m[1] * m[14] -
				 m[4] * m[2] * m[13] -	m[12] * m[1] * m[6] +	m[12] * m[2] * m[5]) * one_over_det;
		r[15] = (m[0] * m[5] * m[10] -m[0] * m[6] * m[9] - m[4] * m[1] * m[10] +
				 m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5]) * one_over_det;

		return r;
	}

	Matrix4 Matrix4::transpose(void) const
	{
		Matrix4 r;
		int t = 0;
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				r[t++] = m[j * 4 + i];
			}
		}
		return r;
	}

	float Matrix4::determinant(void) const
	{
		// based on http://www.euclideanspace.com/maths/algebra/matrix/functions/determinant/fourD/index.htm
		return m[12] * m[9] * m[6] * m[3] - m[8] * m[13] * m[6] * m[3]-
				m[12] * m[5] * m[10] * m[3]+m[4] * m[13] * m[10] * m[3]+
				m[8] * m[5] * m[14] * m[3]-m[4] * m[9] * m[14] * m[3]-
				m[12] * m[9] * m[2] * m[7]+m[8] * m[13] * m[2] * m[7]+
				m[12] * m[1] * m[10] * m[7]-m[0] * m[13] * m[10] * m[7]-
				m[8] * m[1] * m[14] * m[7]+m[0] * m[9] * m[14] * m[7]+
				m[12] * m[5] * m[2] * m[11]-m[4] * m[13] * m[2] * m[11]-
				m[12] * m[1] * m[6] * m[11]+m[0] * m[13] * m[6] * m[11]+
				m[4] * m[1] * m[14] * m[11]-m[0] * m[5] * m[14] * m[11]-
				m[8] * m[5] * m[2] * m[15]+m[4] * m[9] * m[2] * m[15]+
				m[8] * m[1] * m[6] * m[15]-m[0] * m[9] * m[6] * m[15]-
				m[4] * m[1] * m[10] * m[15]+m[0] * m[5] * m[10] * m[15];
	}

	void Matrix4::translate(const Vector3 & v)
	{
		m[12] = m[ 0] * v.x + m[ 4] * v.y + m[ 8] * v.z + m[12];
		m[13] = m[ 1] * v.x + m[ 5] * v.y + m[ 9] * v.z + m[13];
		m[14] = m[ 2] * v.x + m[ 6] * v.y + m[10] * v.z + m[14];
		m[15] = m[ 3] * v.x + m[ 7] * v.y + m[11] * v.z + m[15];
	}

	void Matrix4::setup_translation(const Vector3& v)
	{
		m[ 0] = 1.0f; m[ 1] = 0.0f; m[ 2] = 0.0f; m[ 3] = 0.0f;
		m[ 4] = 0.0f; m[ 5] = 1.0f; m[ 6] = 0.0f; m[ 7] = 0.0f;
		m[ 8] = 0.0f; m[ 9] = 0.0f; m[10] = 1.0f; m[11] = 0.0f;
		m[12] = v.x;  m[13] = v.y;  m[14] = v.z;  m[15] = 1.0f;
	}

	void Matrix4::setup_scale(const Vector3& v)
	{
		m[0 ] = v.x;  m[ 1] = 0.0f; m[ 2] = 0.0f; m[ 3] = 0.0f;
		m[4 ] = 0.0f; m[ 5] = v.y;  m[ 6] = 0.0f; m[ 7] = 0.0f;
		m[8 ] = 0.0f; m[ 9] = 0.0f; m[10] = v.z;  m[11] = 0.0f;
		m[12] = 0.0f; m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;
	}

	void Matrix4::setup_scale(float k)
	{
		m[ 0] = k;	  m[ 1] = 0.0f; m[ 2] = 0.0f; m[ 3] = 0.0f;
		m[ 4] = 0.0f; m[ 5] = k;    m[ 6] = 0.0f; m[ 7] = 0.0f;
		m[ 8] = 0.0f; m[ 9] = 0.0f; m[10] = k;    m[11] = 0.0f;
		m[12] = 0.0f; m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;
	}

	void Matrix4::setup_orthographic(float left, float top, float right, float bottom, float near, float far)
	{
		m[0] = 2.0f / (right - left); m[1] = 0.0f; m[2] = 0.0f; m[3] = 0.0f;
		m[4] = 0.0f; m[5] = 2.0f / (top - bottom); m[6] = 0.0f; m[7] = 0.0f;
		m[8] = 0.0f; m[9] = 0.0f; m[10] = -2.0f / (far - near); m[11] = 0.0f;
		m[12] = -(right + left) / (right - left); m[13] = -(top + bottom) / (top - bottom); m[14] = -(far + near) / (far - near); m[15] = 1.0f;
	}

	void Matrix4::setup_perspective(float default_fov, float aspect, float near, float far)
	{
		float t = tan(0.5f * deg_to_rad(default_fov)) * near;
		float b = -t;
		float r = aspect * t; 
		float l = -r;
		setup_frustrum(l, r, b, t, near, far);
	}

	void Matrix4::setup_frustrum(float left, float right, float bottom, float top, float near, float far)
	{
		float tmp1 = 2.0f * near;
		float tmp2 = right - left;
		float tmp3 = top - bottom;
		float tmp4 = far - near;
		m[0] = tmp1 / tmp2; m[1] = 0.0f; m[2] = 0.0f; m[3] = 0.0f;
		m[4] = 0.0f; m[5] = tmp1 / tmp3; m[6] = 0.0f; m[7] = 0.0f;
		m[8] = (right + left) / tmp2; m[9] = (top + bottom) / tmp3; m[10] = (-far - near) / tmp4; m[11] = -1.0f;
		m[12] = 0.0f; m[13] = 0.0f; m[14] = (-tmp1 * far) / tmp4; m[15] = 0.0f;
	}

	void Matrix4::extract_rotation_axis(Vector3 & left, Vector3 & up, Vector3 & dir) const
	{
		left.x = m[0]; left.y = m[1]; left.z = m[2];
		up.x = m[4]; up.y = m[5]; up.z = m[6];
		dir.x = m[8]; dir.y = m[9]; dir.z = m[10];
	}

	void Matrix4::extract_translation(Vector3 & v) const
	{
		v.x = m[12]; v.y = m[13]; v.z = m[14];
	}

	void Matrix4::setup_rotation(const Vector3 & axis, float theta)
	{
		// quick sanity check to make sure they passed in a unit vector 
		// to specify the axis
		assert(abs(axis * axis - 1.0f) < .01f);

		// get sin and cosine of rotation angle
		float s, c;
		sin_cos(s, c, theta);

		// compute 1 - cos(theta) and some common subexpressions
		float a = 1.0f - c;
		float ax = a * axis.x;
		float ay = a * axis.y;
		float az = a * axis.z;

		// set the matrix elements. there is still a little more
		// opportunity for optimization due to the many common
		// subexpressions. We'll ket the compiler handle that...
		m[ 0] = ax * axis.x + c;		  m[ 1] = ax * axis.y + axis.z * s;	m[ 2] = ax * axis.z - axis.y * s; m[ 3] = 0.0f;
		m[ 4] = ay * axis.x - axis.z * s; m[ 5] = ay * axis.y + c;			m[ 6] = ay * axis.z + axis.x * s; m[ 7] = 0.0f;
		m[ 8] = az * axis.x + axis.y * s; m[ 9] = az * axis.y - axis.x * s;	m[10] = az * axis.z + c;		  m[11] = 0.0f;
		m[12] = 0.0f;					  m[13] = 0.0f;						m[14] = 0.0f;					  m[15] = 1.0f;
	}

	void Matrix4::setup_rotation(const Quaternion& q)
	{
		// compute a few values to optimize common subexpressions
		float ww = 2.0f * q.w;
		float xx = 2.0f * q.x;
		float yy = 2.0f * q.y;
		float zz = 2.0f * q.z;
		m[ 0] = 1.0f - yy * q.y - zz * q.z; m[ 1] = xx * q.y + ww * q.z;		m[ 2] = xx*q.z - ww*q.y;			m[ 3] = 0.0f;
		m[ 4] = xx * q.y - ww * q.z;		m[ 5] = 1.0f - xx * q.x - zz * q.z; m[ 6] = yy*q.z + ww*q.x;			m[ 7] = 0.0f;
		m[ 8] = xx * q.z + ww * q.y;		m[ 9] = yy * q.z - ww * q.x;		m[10] = 1.0f - xx * q.x - yy * q.y; m[11] = 0.0f;
		m[12] = 0.0f;						m[13] = 0.0f;						m[14] = 0.0f;						m[15] = 1.0f;
	}

	void Matrix4::setup_rotation(const Vector3 & left, const Vector3 & up, const Vector3 & dir)
	{
		// build up rotation matrix from rotation axis
		m[0] = left.x;	m[1] = left.y;	m[2] = left.z;	m[3] = 0.0f;
		m[4] = up.x;	m[5] = up.y;	m[6] = up.z;	m[7] = 0.0f;
		m[8] = dir.x;	m[9] = dir.y;	m[10] = dir.z;	m[11] = 0.0f;
		m[12] = 0.0f;	m[13] = 0.0f;	m[14] = 0.0f;	m[15] = 1.0f;
	}

	Vector3 operator*(const Vector3& p, const Matrix4& m)
	{
		return Vector3(
			p.x*m.m[0] + p.y*m.m[4] + p.z*m.m[8] + m.m[12],
			p.x*m.m[1] + p.y*m.m[5] + p.z*m.m[9] + m.m[13],
			p.x*m.m[2] + p.y*m.m[6] + p.z*m.m[10] + m.m[14]
		);
	}

	Matrix4 operator*(const Matrix4& a, const Matrix4& b)
	{
		Matrix4 r;

		r.m[0] = a.m[0]*b.m[0] + a.m[4]*b.m[1] + a.m[8]*b.m[2] + a.m[12]*b.m[3];
		r.m[1] = a.m[1]*b.m[0] + a.m[5]*b.m[1] + a.m[9]*b.m[2] + a.m[13]*b.m[3];
		r.m[2] = a.m[2]*b.m[0] + a.m[6]*b.m[1] + a.m[10]*b.m[2] + a.m[14]*b.m[3];
		r.m[3] = a.m[3]*b.m[0] + a.m[7]*b.m[1] + a.m[11]*b.m[2] + a.m[15]*b.m[3];

		r.m[4] = a.m[0]*b.m[4] + a.m[4]*b.m[5] + a.m[8]*b.m[6] + a.m[12]*b.m[7];
		r.m[5] = a.m[1]*b.m[4] + a.m[5]*b.m[5] + a.m[9]*b.m[6] + a.m[13]*b.m[7];
		r.m[6] = a.m[2]*b.m[4] + a.m[6]*b.m[5] + a.m[10]*b.m[6] + a.m[14]*b.m[7];
		r.m[7] = a.m[3]*b.m[4] + a.m[7]*b.m[5] + a.m[11]*b.m[6] + a.m[15]*b.m[7];

		r.m[8] = a.m[0]*b.m[8] + a.m[4]*b.m[9] + a.m[8]*b.m[10] + a.m[12]*b.m[11];
		r.m[9] = a.m[1]*b.m[8] + a.m[5]*b.m[9] + a.m[9]*b.m[10] + a.m[13]*b.m[11];
		r.m[10] = a.m[2]*b.m[8] + a.m[6]*b.m[9] + a.m[10]*b.m[10] + a.m[14]*b.m[11];
		r.m[11] = a.m[3]*b.m[8] + a.m[7]*b.m[9] + a.m[11]*b.m[10] + a.m[15]*b.m[11];

		r.m[12] = a.m[0]*b.m[12] + a.m[4]*b.m[13] + a.m[8]*b.m[14] + a.m[12]*b.m[15];
		r.m[13] = a.m[1]*b.m[12] + a.m[5]*b.m[13] + a.m[9]*b.m[14] + a.m[13]*b.m[15];
		r.m[14] = a.m[2]*b.m[12] + a.m[6]*b.m[13] + a.m[10]*b.m[14] + a.m[14]*b.m[15];
		r.m[15] = a.m[3]*b.m[12] + a.m[7]*b.m[13] + a.m[11]*b.m[14] + a.m[15]*b.m[15];

		return r;
	}

	Matrix4 operator*(float s, const Matrix4& m)
	{
		Matrix4 r;
		for (int i = 0; i < 16; i++)
			r.m[i] = s * m.m[i];
		return r;
	}

	Vector3& operator*=(Vector3& p, const Matrix4& m)
	{
		p = p * m;
		return p;
	}

	Matrix4& operator *=(Matrix4& a, const Matrix4& b)
	{
		Matrix4 r;

		r.m[0] = a.m[0]*b.m[0] + a.m[4]*b.m[1] + a.m[8]*b.m[2] + a.m[12]*b.m[3];
		r.m[1] = a.m[1]*b.m[0] + a.m[5]*b.m[1] + a.m[9]*b.m[2] + a.m[13]*b.m[3];
		r.m[2] = a.m[2]*b.m[0] + a.m[6]*b.m[1] + a.m[10]*b.m[2] + a.m[14]*b.m[3];
		r.m[3] = a.m[3]*b.m[0] + a.m[7]*b.m[1] + a.m[11]*b.m[2] + a.m[15]*b.m[3];

		r.m[4] = a.m[0]*b.m[4] + a.m[4]*b.m[5] + a.m[8]*b.m[6] + a.m[12]*b.m[7];
		r.m[5] = a.m[1]*b.m[4] + a.m[5]*b.m[5] + a.m[9]*b.m[6] + a.m[13]*b.m[7];
		r.m[6] = a.m[2]*b.m[4] + a.m[6]*b.m[5] + a.m[10]*b.m[6] + a.m[14]*b.m[7];
		r.m[7] = a.m[3]*b.m[4] + a.m[7]*b.m[5] + a.m[11]*b.m[6] + a.m[15]*b.m[7];

		r.m[8] = a.m[0]*b.m[8] + a.m[4]*b.m[9] + a.m[8]*b.m[10] + a.m[12]*b.m[11];
		r.m[9] = a.m[1]*b.m[8] + a.m[5]*b.m[9] + a.m[9]*b.m[10] + a.m[13]*b.m[11];
		r.m[10] = a.m[2]*b.m[8] + a.m[6]*b.m[9] + a.m[10]*b.m[10] + a.m[14]*b.m[11];
		r.m[11] = a.m[3]*b.m[8] + a.m[7]*b.m[9] + a.m[11]*b.m[10] + a.m[15]*b.m[11];

		r.m[12] = a.m[0]*b.m[12] + a.m[4]*b.m[13] + a.m[8]*b.m[14] + a.m[12]*b.m[15];
		r.m[13] = a.m[1]*b.m[12] + a.m[5]*b.m[13] + a.m[9]*b.m[14] + a.m[13]*b.m[15];
		r.m[14] = a.m[2]*b.m[12] + a.m[6]*b.m[13] + a.m[10]*b.m[14] + a.m[14]*b.m[15];
		r.m[15] = a.m[3]*b.m[12] + a.m[7]*b.m[13] + a.m[11]*b.m[14] + a.m[15]*b.m[15];

		// store result back into a
		a.m[0] = r.m[0]; a.m[1] = r.m[1]; a.m[2] = r.m[2]; a.m[3] = r.m[3];
		a.m[4] = r.m[4]; a.m[5] = r.m[5]; a.m[6] = r.m[6]; a.m[7] = r.m[7];
		a.m[8] = r.m[8]; a.m[9] = r.m[9]; a.m[10] = r.m[10]; a.m[11] = r.m[11];
		a.m[12] = r.m[12]; a.m[13] = r.m[13]; a.m[14] = r.m[14]; a.m[15] = r.m[15];

		return a;
	}

	Matrix4& operator*=(Matrix4& m, const float s)
	{
		for (int i = 0; i < 16; i++)
			m.m[i] *= s;
		return m;
	}
}