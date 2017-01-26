#include "stdafx.h"
#include "roomcamera.h"

namespace dukat
{
	const float RoomCamera::jump_acceleration = 2.0f;
	const float RoomCamera::gravity = 3.0f;
    
    void RoomCamera::update(float delta) 
    {
        // jumping
		if (accel != 0.0f)
		{
			transform.position.y += accel * delta;
			accel -= gravity * delta;

			if (transform.position.y < 1.0f)
			{
				transform.position.y = 1.0f;
				accel = 0.0f;
			}
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