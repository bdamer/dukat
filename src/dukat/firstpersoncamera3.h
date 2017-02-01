#pragma once

#include <memory>
#include "camera3.h"
#include "quaternion.h"
#include "vector3.h"

namespace dukat
{
	class GameBase;

	class FirstPersonCamera3 : public Camera3
	{
	private:
		const float yaw_speed;
		const float pitch_speed;
		float movement_speed;
		float yaw, pitch;
		GameBase* game;
		Vector3 last_movement;

	public:
		bool mouse_look;

		FirstPersonCamera3(Window* window, GameBase* game) : Camera3(window), game(game), mouse_look(true), 
			yaw(0.0f), pitch(0.0f), yaw_speed(1.0f), pitch_speed(1.0f), movement_speed(2.0f) { }
		~FirstPersonCamera3(void) { }

		virtual void update(float delta);

		// Changes distance between camera position and target.
		float get_distance(void) const { return 0.0f; }
		void set_distance(float distance) { }
		void set_movement_speed(float speed) { movement_speed = speed; }
		float get_movement_speed(void) const { return movement_speed; }
	};
}