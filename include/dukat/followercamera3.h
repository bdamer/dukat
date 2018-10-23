#pragma once

#include "camera3.h"

namespace dukat 
{
	class PlayerController;

	template <typename T>
	class FollowerCamera3 : public Camera3
	{
	private:
		// Spring and damping factors
		// These are dependent on the amount of change allowed by the camera controller
		float spring_factor;
		float damping_factor;
		// Target entity
		T* target;
		// Offset between camera and target position.
		Vector3 target_offset;
		// Vector between camera and focus point.
		Vector3 focus_offset;
		// Actual focus vector
		Vector3 focus_actual;
		// Velocity of change to focus
		Vector3 change_vel;

	public:
		// Creates a new camera following target. target_offset is the vector from camera to target.
		FollowerCamera3(GameBase* game, T* target, const Vector3& target_offset, const Vector3& focus_offset);
		~FollowerCamera3(void) { }
		void update(float delta);

		// Changes distance between camera position and target.
		float get_distance(void) const { return target_offset.mag(); }
		void set_distance(float distance) { target_offset.normalize(); target_offset *= distance; }
		void set_spring(float spring) { spring_factor = spring; }
		void set_damping(float damping) { damping_factor = damping; }
	};

	template <typename T>
	FollowerCamera3<T>::FollowerCamera3(GameBase* game, T* target, const Vector3& target_offset, const Vector3& focus_offset)
		: Camera3(game->get_window()), target(target), target_offset(target_offset), focus_offset(focus_offset), 
		  change_vel({ 0.0f, 0.0f, 0.0f }), spring_factor(0.15f), damping_factor(0.025f)
	{
		// Initial setup
		const auto& target_transform = target->get_transform();
		transform.position = target_transform.position - target_offset * target_transform.mat_rot;
		focus_actual = (target_transform.position + (focus_offset - target_offset) * target_transform.mat_rot) - transform.position;
	}

	template <typename T>
	void FollowerCamera3<T>::update(float delta)
	{
		if (target == nullptr)
			return;

		const auto& target_transform = target->get_transform();

		// Update position based on rotated camera offset
		transform.position = target_transform.position - target_offset * target_transform.mat_rot;
		// Determine ideal vectors from camera to focus point
		dukat::Vector3 ideal = (target_transform.position + (focus_offset - target_offset) * target_transform.mat_rot) - transform.position;
		// Determine displacement between current and ideal vector		
		dukat::Vector3 displacement = ideal - focus_actual;
		// Compute accelleration based on difference and existing change velocity
		dukat::Vector3 springAccel = (displacement * spring_factor) - (change_vel * damping_factor);
		// apply change to current focus vector
		change_vel += springAccel;
		focus_actual += change_vel * delta;
		transform.dir = focus_actual;

		// Compute remaining axes (order of cross product operands matters!)
		transform.dir.normalize();
		transform.right = cross_product(transform.dir, target_transform.up);
		transform.right.normalize();
		transform.up = cross_product(transform.right, transform.dir);
		transform.up.normalize();

		Camera3::update(delta);
	}
}