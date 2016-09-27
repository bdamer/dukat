#include "stdafx.h"
#include "transform3.h"
#include "mathutil.h"

namespace dukat
{
	ExtendedTransform3::ExtendedTransform3(void) : Transform3(), rot(Quaternion::unit)
	{
		mat_rot.identity();
	}

	ExtendedTransform3::ExtendedTransform3(const Transform3& t) : Transform3(t) 
	{
		// Matrix-to-quaternion conversion can cycle between equivalent
		// forms which contain different values; therefore, jump over 
		// one of these cycles.
		mat_rot.setup_rotation(left, up, dir);
		rot.from_matrix(mat_rot);
		mat_rot.setup_rotation(rot);
		rot.from_matrix(mat_rot);
	}

	void ExtendedTransform3::update(void)
	{
		// TODO: only compute if values were changed

		// Compute model matrix
		mat_trans.setup_translation(position);
		mat_rot.setup_rotation(rot);		
		mat_scale.setup_scale(scale);
		mat_model = mat_trans * mat_rot * mat_scale;
	
		mat_rot.extract_rotation_axis(left, up, dir);
	}
}
