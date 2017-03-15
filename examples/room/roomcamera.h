#pragma once

#include <dukat/firstpersoncamera3.h>
#include <dukat/gamebase.h>

namespace dukat
{
    class RoomCamera : public FirstPersonCamera3
    {
    private:
		static const float jump_acceleration;
		static const float gravity;
		float accel;

    public:
		RoomCamera(Window* window, GameBase* game) : FirstPersonCamera3(game), accel(0.0f) { }
		~RoomCamera(void) { }

		void jump(void);
		void update(float delta);
    };
}