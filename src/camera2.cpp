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

	Camera2::Camera2(GameBase* game) : window(game->get_window()), 
		near_clip(default_near_clip), far_clip(default_far_clip), 
		mag_factor(1.f), effect(nullptr), handler(nullptr)
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

	void Camera2::resize(int window_width, int window_height)
	{
		if (handler != nullptr)
		{
			handler(this, window);
		}
		else
		{
			transform.dimension.x = static_cast<float>(window_width);
			transform.dimension.y = static_cast<float>(window_height);
		}
		log->debug("Resizing camera to: {}x{}", static_cast<int>(transform.dimension.x), static_cast<int>(transform.dimension.y));

		aspect_ratio = transform.dimension.x / transform.dimension.y;

		mag_factor = std::round(static_cast<float>(window_width) / transform.dimension.x);
		log->debug("Camera magnification: {}", mag_factor);

		// Set up projection matrix to physical dimensions of window
		transform.mat_proj_orth.setup_orthographic(-static_cast<float>(window_width) / 2.0f, -static_cast<float>(window_height) / 2.0f,
			static_cast<float>(window_width) / 2.0f, static_cast<float>(window_height) / 2.0f, near_clip, far_clip);
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
		transform.mat_view.setup_translation(Vector3(-transform.position.x * mag_factor, -transform.position.y * mag_factor, 0.0f));
	}

	Camera2::resize_handler fixed_camera(int width, int height)
	{
		return [width, height](Camera2* camera, Window* window) {
			camera->transform.dimension.x = static_cast<float>(width);
			camera->transform.dimension.y = static_cast<float>(height);
		};
	}

	Camera2::resize_handler fixed_width_camera(int width)
	{
		return [width](Camera2* camera, Window* window) {
			camera->transform.dimension.x = static_cast<float>(width);
			// adjust height to nearest multiple of 2
			auto height = static_cast<int>(std::round(static_cast<float>(width) / window->get_aspect_ratio()));
			height += height % 2;
			camera->transform.dimension.y = static_cast<float>(height);
		};
	}

	Camera2::resize_handler fixed_height_camera(int height)
	{
		return [height](Camera2* camera, Window* window) {
			const auto ratio = static_cast<float>(window->get_height()) / static_cast<float>(window->get_width());
			// adjust width to nearest multiple of 2
			auto width = static_cast<int>(std::round(static_cast<float>(height) / ratio));
			width += width % 2; 
			camera->transform.dimension.x = static_cast<float>(width);
			camera->transform.dimension.y = static_cast<float>(height);
		};
	}

	Camera2::resize_handler limited_height_camera(int min_height, int max_height)
	{
		return [min_height, max_height](Camera2* camera, Window* window) {
			const auto ratio = static_cast<float>(window->get_height()) / static_cast<float>(window->get_width());
			const auto height = compute_ideal_height(window->get_height(), min_height, max_height);
			// adjust width to nearest multiple of 2
			auto width = static_cast<int>(std::round(static_cast<float>(height) / ratio));
			width += width % 2;
			camera->transform.dimension.x = static_cast<float>(width);
			camera->transform.dimension.y = static_cast<float>(height);
		};
	}

	int compute_ideal_height(int screen_height, int min_height, int max_height)
	{
		// find smallest value greater than min_height that evenly divides height
		for (auto h = min_height; h <= max_height; h += 2)
		{
			if (screen_height % h == 0)
				return h;
		}
		return -1;
	}
}