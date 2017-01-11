#include "stdafx.h"
#include "firstpersoncamera.h"
#include "roomapp.h"
#include <dukat/devicemanager.h>
#include <dukat/inputdevice.h>
#include <dukat/mathutil.h>

namespace dukat
{
	const float FirstPersonCamera::yaw_speed = 1.0f;
	const float FirstPersonCamera::pitch_speed = 1.0f;
	const float FirstPersonCamera::movement_speed = 2.0f;
	const float FirstPersonCamera::jump_acceleration = 2.0f;
	const float FirstPersonCamera::gravity = 3.0f;

	FirstPersonCamera::FirstPersonCamera(Window* window, Game* game) : Camera3(window), game(game), accel(0.0f)
	{
	}

	void FirstPersonCamera::update(float delta)
	{
		auto dev = game->get_devices()->active;

		// rotating by negative amount since rotation is clockwise around axis
		if (dev->rx != 0.0f)
		{
			yaw += -dev->rx * yaw_speed * delta;
		}
		if (dev->ry != 0.0f)
		{
			pitch += -dev->ry * pitch_speed * delta;
			// limit range of pitch
			float range = pi_over_two - pi_over_eight;
			clamp(pitch, -range, range);
		}

		// Rotate transform
		Quaternion qy, qx;
		qy.set_to_rotate_y(yaw);
		qx.set_to_rotate_x(pitch);
		Matrix4 mat_rot;
		mat_rot.setup_rotation(qx * qy);
		mat_rot.extract_rotation_axis(transform.right, transform.up, transform.dir);
		transform.right = -transform.right;

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
		// movement on ground
		else
		{
			// create direction vector without y-component to stay at the same level
			Vector3 dir(transform.dir.x, 0.0f, transform.dir.z);
			dir.normalize();

			last_movement = { 0.0f, 0.0f, 0.0f };

			// Movement along dir
			if (dev->ly > 0.0f)
			{
				last_movement += movement_speed * delta * dir;
			}
			else if (dev->ly < 0.0f)
			{
				last_movement -= movement_speed * delta * dir;
			}

			// Strafing
			auto left = cross_product(Vector3::unit_y, dir);
			left.normalize();
			if (dev->lx > 0.25f)
			{
				last_movement -= left * delta;
			}
			else if (dev->lx < -0.25f)
			{
				last_movement += left * delta;
			}

		}

		transform.position += last_movement;

		Camera3::update(delta);
	}

	void FirstPersonCamera::jump(void)
	{
		if (accel == 0.0f)
		{
			accel = jump_acceleration;
		}
	}
}