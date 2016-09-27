#pragma once

namespace dukat
{
	class Quaternion;

	// Class representing a heading-pitch-bank Euler angle triple.
	class EulerAngles
	{
	public:
		// radians
		float heading;
		float pitch;
		float bank;

		// Default constructor does nothing
		EulerAngles(void) { };
		// Construct from three values
		EulerAngles(float h, float p, float b) : heading(h), pitch(p), bank(b) { }
		
		// set to identity triple (all zeros)
		void identity() { pitch = bank = heading = 0.0f; }

		// Set the Euler angle triple to its "canonical" value. This does not change
		// the meaning of the Euler angles as a representation of orientation in 3D,
		// but if the angles are for other purposes such as angulgar velocities, etc.,
		// then the operation might not be valid.
		void canonize();

		// Convert to quaternion to Euler angle format. The input quaternion
		// is assumed to perform the rotation from object-to-inertial
		// or inertial-to-object, as indicated.
		void from_object_to_inertial_quaternion(const Quaternion& q);
		void from_inertial_to_object_quaternion(const Quaternion& q);

	};
}