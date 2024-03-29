#pragma once

#include <dukat/dukat.h>

namespace dukat
{
    class RoomCamera : public FirstPersonCamera3
    {
    private:
		static const float jump_acceleration;
		static const float gravity;
		float accel;
		bool walking;

    public:
		RoomCamera(GameBase* game) : FirstPersonCamera3(game), accel(0.0f), walking(false) { }
		~RoomCamera(void) { }

		void jump(void);
		void update(float delta);
    };
}