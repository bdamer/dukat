#include "stdafx.h"
#include <dukat/eulerangles.h>
#include <dukat/mathutil.h>
#include <dukat/quaternion.h>
#include <dukat/vector3.h>

namespace dukat
{
	void EulerAngles::canonize()
	{
		// First, rap pitch in range -PI...PI
		pitch = wrap_pi(pitch);

		// Now, check for the "back side" of the matrix pitch outside 
		// the canonical range of -PI/2 ... PI/2
		if (pitch < -pi_over_two)
		{
			pitch = -pi - pitch;
			heading += pi;
			bank += pi;
		}
		else if (pitch > pi_over_two)
		{
			pitch = pi - pitch;
			heading += pi;
			bank += pi;
		}

		// now check for the gimbel lock case (within a slight tolerance)
		if (fabs(pitch) > pi_over_two - 1e-4)
		{
			// we are in gimbel lock. assign all rotation
			// about the vertical axis to heading
			heading += bank;
			bank = 0.0f;
		}
		else
		{
			// not in gimbel lock. wrap the bank angle in
			// canonical range
			bank = wrap_pi(bank);
		}

		// wrap heading in canonical range
		heading = wrap_pi(heading);
	}

	void EulerAngles::from_object_to_inertial_quaternion(const Quaternion& q)
	{
		// extract sin(pitch)
		float sp = -2.0f * (q.y * q.z - q.w * q.x);
		// check for gimbel lock, giving slight tolerance for numerical imprecision
		if (fabs(sp) > 0.9999f)
		{
			// looking straight up or down
			pitch = pi_over_two * sp;
			// compute heading, slam bank to zero
			heading = atan2(-q.x*q.z + q.w*q.y, 0.5f - q.y*q.y - q.z*q.z);
			bank = 0.0f;
		}
		else
		{
			// compute angles. we don't have to use the "safe" asin
			// function because we already checked for range errors when 
			// checking for Gimbel lock.
			pitch = asin(sp);
			heading = atan2(q.x * q.z + q.w * q.y, 0.5f - q.x * q.x - q.y * q.y);
			bank = atan2(q.x * q.y + q.w * q.z, 0.5f - q.x * q.x - q.z * q.z);
		}
	}

	void EulerAngles::from_inertial_to_object_quaternion(const Quaternion& q)
	{
		// extract sin(pitch)
		float sp = -2.0f * (q.y * q.z + q.w * q.x);
		// check for Gimbel lock, giving slight tolerance for numerical imprecision
		if (fabs(sp) > 0.9999f)
		{
			// looking straight up or down
			pitch = pi_over_two * sp;
			// compute heading, slam bank to zero
			heading = atan2(-q.x * q.z - q.w * q.y, 0.5f - q.y * q.y - q.z * q.z);
			bank = 0.0f;
		}
		else
		{
			// compute angles. we don't have to use the "safe" asin
			// function because we already checked for range errors when 
			// checking for Gimbel lock.
			pitch = asin(sp);
			heading = atan2(q.x * q.z - q.w * q.y, 0.5f - q.x * q.x - q.y * q.y);
			bank = atan2(q.x * q.y - q.w * q.z, 0.5f - q.x * q.x - q.z * q.z);
		}
	}
}