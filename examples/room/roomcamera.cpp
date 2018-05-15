#include "stdafx.h"
#include "roomcamera.h"

namespace dukat
{
	const float RoomCamera::jump_acceleration = 2.0f;
	const float RoomCamera::gravity = 3.0f;
    
    void RoomCamera::update(float delta) 
    {
        // jumping
		bool jumping = accel != 0.0f;
		if (jumping)
		{
			transform.position.y += accel * delta;
			accel -= gravity * delta;

			if (transform.position.y < 1.0f)
			{
				transform.position.y = 1.0f;
				accel = 0.0f;
			}
		}

		// Play / stop footsteps sample
		auto dev = game->get_devices()->active;
		auto should_walk = !jumping && (dev->lx != 0.0f || dev->ly != 0.0f);
		if (should_walk && !walking)
		{
			auto sample = game->get_samples()->get_sample("footsteps.ogg");
			game->get_audio()->play_sample(sample, 0, -1);
			walking = true;
		}
		else if (!should_walk && walking)
		{
			game->get_audio()->stop_sample(0);
			walking = false;
		}

        FirstPersonCamera3::update(delta);
    }

    void RoomCamera::jump(void)
	{
		if (accel == 0.0f)
		{
			accel = jump_acceleration;
		}
	}
}