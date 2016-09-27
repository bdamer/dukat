#pragma once

#include "quaternion.h"
#include "vector3.h"
#include "matrix4.h"

namespace dukat
{
	// Static transform
	struct Transform3
	{
		// Translation
		Vector3 position;
		// Rotation
		Vector3 dir;
		Vector3 up;
		Vector3 left;
		// Scaling
		Vector3 scale;

		Transform3(void)
			: position(Vector3::origin), dir(Vector3::unit_z), up(Vector3::unit_y), left(Vector3::unit_x), scale(1.0f, 1.0f, 1.0f) { }
	};

	// Extended transform including matrices
	struct ExtendedTransform3 : public Transform3
	{
		Matrix4 mat_trans;
		Quaternion rot;
		Matrix4 mat_rot;
		Matrix4 mat_scale;
		// Combined model matrix
		Matrix4 mat_model;
		
		ExtendedTransform3(void);
		ExtendedTransform3(const Transform3& t);
		void update(void);
	};
}
