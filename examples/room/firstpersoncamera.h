#pragma once

#include <memory>
#include <dukat/camera3.h>
#include <dukat/quaternion.h>

namespace dukat
{
	class Game;

	class FirstPersonCamera : public Camera3
	{
	private:
		static const float yaw_speed;
		static const float pitch_speed;
		static const float movement_speed;
		static const float jump_acceleration;
		static const float gravity;
		float yaw, pitch;
		float accel;
		Game* game;
		Vector3 last_movement;

	public:
		FirstPersonCamera(Window* window, Game* game);
		~FirstPersonCamera(void) { }

		void update(float delta);
		void jump(void);

		// Changes distance between camera position and target.
		float get_distance(void) const { return 0.0f; }
		void set_distance(float distance) { }
	};
}