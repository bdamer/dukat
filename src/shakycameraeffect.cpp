#include "stdafx.h"
#include <dukat/camera2.h>
#include <dukat/shakycameraeffect.h>
#include <dukat/mathutil.h>

namespace dukat
{
	void ShakyCameraEffect::update_transform(float delta, CameraTransform2 & tx)
	{
		if (next_jump <= 0.0f)
		{
			tx.position.y += sign_y * randf(min_range, max_range);
			next_jump = frequency;
			sign_y *= -1;
		}
		else
		{
			next_jump -= delta;
		}
		CameraEffect2::update_transform(delta, tx);
	}
}