#include "stdafx.h"
#include "firstpersoncamera3.h"
#include "devicemanager.h"
#include "inputdevice.h"
#include "mathutil.h"
#include "gamebase.h"

namespace dukat
{
	void FirstPersonCamera3::update(float delta)
	{
		auto dev = game->get_devices()->active;

		// rotating by negative amount since rotation is clockwise around axis
		if (mouse_look)
		{
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
			last_movement -= movement_speed * delta * left;
		}
		else if (dev->lx < -0.25f)
		{
			last_movement += movement_speed * delta * left;
		}

		transform.position += last_movement;

		Camera3::update(delta);
	}
}