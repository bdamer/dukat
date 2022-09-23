#pragma once

#include "matrix4.h"
#include "recipient.h"
#include "vector2.h"
#include "window.h"
#include "cameraeffect2.h"
#include "aabb2.h"

namespace dukat
{
	class GameBase;
	
	struct CameraTransform2
	{
		Matrix4 mat_proj_orth; // orthographic projection matrix
		Matrix4 mat_view; // view matrix 
		Vector2 position; // Position in world space
		Vector2 dimension; // Width / height in world space
		CameraTransform2(void) : position(Vector2::origin) { }
	};

	class Camera2
	{
	public:
		typedef std::function<void(Camera2 * camera, Window * window)> resize_handler;

	protected:
		static const float default_near_clip;
		static const float default_far_clip;
		Window* window;
		float aspect_ratio;
		float near_clip;
		float far_clip;
		float mag_factor;
		std::unique_ptr<CameraEffect2> effect;
		resize_handler handler;

	public:
		CameraTransform2 transform;

		// Creates a new camera for a given window.
		Camera2(GameBase* game);
		virtual ~Camera2(void);

		void resize(int width, int height);
		void set_clip(float near_clip, float far_clip) { this->near_clip = near_clip; this->far_clip = far_clip; }
		void refresh(void) { resize(window->get_width(), window->get_height()); }
		float get_aspect_ratio(void) const { return aspect_ratio; }
		float get_mag_factor(void) const { return mag_factor; }
		void set_effect(std::unique_ptr<CameraEffect2> effect);
		void set_resize_handler(const resize_handler& func) { handler = func; }

		// Returns bounding box for camera in world space. 
		// If parallax value is provided, will adjust camera
		// position accordingly.
		AABB2 get_bb(float parallax = 1.0f);

		// Updates the camera's view matrix. Subclasses of camera should update
		// the camera axes and call this method to update the view matrix.
		virtual void update(float delta);
	};

	// Camera resize handlers
	Camera2::resize_handler fixed_camera(int width, int height);
	Camera2::resize_handler fixed_width_camera(int width);
	Camera2::resize_handler fixed_height_camera(int height);
	Camera2::resize_handler limited_height_camera(int min_height, int max_height);

	// Computes smallest height between min_height and max_height that evenly divides screen_height.
	int compute_ideal_height(int screen_height, int min_height, int max_height);
}
