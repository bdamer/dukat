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
		static const float yaw_speed;
		static const float pitch_speed;
		static const float movement_speed;
		float yaw, pitch;
		GameBase* game;
		Vector3 last_movement;


	public:
		bool mouse_look;

		FirstPersonCamera3(Window* window, GameBase* game) : Camera3(window), game(game), mouse_look(true), yaw(0.0f), pitch(0.0f) { }
		~FirstPersonCamera3(void) { }

		virtual void update(float delta);

		// Changes distance between camera position and target.
		float get_distance(void) const { return 0.0f; }
		void set_distance(float distance) { }
	};
}