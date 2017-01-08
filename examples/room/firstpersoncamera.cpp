#include "stdafx.h"
#include "firstpersoncamera.h"
#include "roomapp.h"
#include <dukat/devicemanager.h>
#include <dukat/inputdevice.h>
#include <dukat/mathutil.h>

namespace dukat
{
	const float FirstPersonCamera::yaw_speed = 2.0f;
	const float FirstPersonCamera::pitch_speed = 2.0f;
	const float FirstPersonCamera::movement_speed = 2.0f;

	FirstPersonCamera::FirstPersonCamera(Window* window, Game* game) : Camera3(window), game(game)
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

		// create direction vector without y-component to stay at the same level
		Vector3 dir(transform.dir.x, 0.0f, transform.dir.z);
		dir.normalize();

		// Movement along dir
		if (dev->ly > 0.0f)
		{
			transform.position += movement_speed * delta * dir;
		}
		else if (dev->ly < 0.0f)
		{
			transform.position -= movement_speed * delta * dir;
		}

		// Strafing
		auto left = cross_product(Vector3::unit_y, dir);
		left.normalize();
		if (dev->lx > 0.0f)
		{
			transform.position -= left * delta;
		}
		else if (dev->lx < 0.0f)
		{
			transform.position += left * delta;
		}

		Camera3::update(delta);
	}
}