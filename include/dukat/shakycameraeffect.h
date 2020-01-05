#pragma once

#include "cameraeffect2.h"

namespace dukat
{
	class ShakyCameraEffect : public CameraEffect2
	{
	private:
		const float frequency;
		const bool vertical;
		const float min_range;
		const float max_range;
		float sign_y;
		float next_jump;

	public:
		ShakyCameraEffect(float duration, bool vertical, float frequency, float min_range, float max_range) 
			: CameraEffect2(duration), vertical(vertical), frequency(frequency), min_range(min_range), max_range(max_range), sign_y(1.0f), next_jump(0.0f) { }
		~ShakyCameraEffect(void) { }

		void update_transform(float delta, CameraTransform2& tx);
	};

}