#include "stdafx.h"
#include <dukat/orbitcamera3.h>
#include <dukat/gamebase.h>
#include <dukat/devicemanager.h>
#include <dukat/inputdevice.h>
#include <dukat/inputrecorder.h>
#include <dukat/quaternion.h>

namespace dukat
{
	OrbitCamera3::OrbitCamera3(GameBase* game, const Vector3& target, float distance, float longitude, float latitude)
		: Camera3(game->get_window()), distance(distance), min_distance(0.0f), max_distance(1000.0f),
		longitude(longitude), latitude(latitude), game(game), look_at(target), rot_speed(1.0f)
	{
	}

	void OrbitCamera3::update(float delta)
	{
		auto dev = game->get_devices()->active;
		if (dev->is_pressed(InputDevice::VirtualButton::Button2))
		{
			longitude -= dev->rx * rot_speed * delta;
			latitude += dev->ry * rot_speed * delta;
			clamp(latitude, -pi_over_two, pi_over_two);
		}

		Quaternion qy, qx;
		qy.set_to_rotate_y(longitude);
		qx.set_to_rotate_x(latitude);
		Matrix4 mat_rot;
		mat_rot.setup_rotation(qx * qy);
		mat_rot.extract_rotation_axis(transform.right, transform.up, transform.dir);
		transform.right = -transform.right;

		transform.position = look_at - transform.dir * distance;

		Camera3::update(delta);
	}
}