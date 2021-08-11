#include "stdafx.h"
#include <dukat/camera2.h>
#include <dukat/cameraeffect2.h>
#include <dukat/log.h>
#include <dukat/vector3.h>
#include <dukat/gamebase.h>

namespace dukat
{
	const float Camera2::default_near_clip = 0.0f;
	const float Camera2::default_far_clip = 1000.0f;

	Camera2::Camera2(GameBase* game) : window(game->get_window()), near_clip(default_near_clip), far_clip(default_far_clip), 
		effect(nullptr), handler(nullptr)
	{
		window->subscribe(this, Events::WindowResized);
	}

	Camera2::~Camera2(void)
	{
		window->unsubscribe(this, Events::WindowResized);
	}

	void Camera2::receive(const Message& msg)
	{
		switch (msg.event)
		{
		case Events::WindowResized:
			resize(*static_cast<const int*>(msg.param1), *static_cast<const int*>(msg.param2));
			break;
		}
	}

	void Camera2::resize(int width, int height)
	{
		if (handler != nullptr)
		{
			handler(this, window);
		}
		else
		{
			transform.dimension.x = static_cast<float>(width);
			transform.dimension.y = static_cast<float>(height);
		}
		log->debug("Resizing camera to: {}x{}", static_cast<int>(transform.dimension.x), static_cast<int>(transform.dimension.y));

		aspect_ratio = transform.dimension.x / transform.dimension.y;
	
		// Set up projection matrix to logical dimensions of window
		transform.mat_proj_orth.setup_orthographic(-transform.dimension.x / 2.0f, -transform.dimension.y / 2.0f,
			transform.dimension.x / 2.0f, transform.dimension.y / 2.0f, near_clip, far_clip);
	}

	void Camera2::set_effect(std::unique_ptr<CameraEffect2> effect)
	{
		if (effect != nullptr)
		{
			log->trace("Starting effect: {}", effect->get_duration());
			this->effect = std::move(effect);
		}
		else
		{
			this->effect = nullptr;
		}
	}

	AABB2 Camera2::get_bb(float parallax)
	{
		const auto camera_pos = transform.position * parallax;
		const auto camera_dim = transform.dimension / 2.0f;
		return AABB2{ camera_pos - camera_dim, camera_pos + camera_dim };
	}

	void Camera2::update(float delta)
	{
		if (effect != nullptr)
		{
			if (effect->is_done())
			{
				log->trace("Effect is done.");
				effect = nullptr;
			}
			else
			{
				effect->update_transform(delta, transform);
			}
		}

		// Rebuild camera / view matrix
		transform.mat_view.setup_translation(Vector3(-transform.position.x, -transform.position.y, 0.0f));
	}
}