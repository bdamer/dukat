#pragma once

namespace dukat
{
	struct CameraTransform2;

	class CameraEffect2
	{
	protected:
		float duration;

	public:
		CameraEffect2(float duration) : duration(duration) { }
		virtual ~CameraEffect2(void) { }

		bool is_done(void) const { return duration <= 0.0f; }

		virtual void update_transform(float delta, CameraTransform2& tx) { duration -= delta; }
	};
}