#pragma once

#include "matrix4.h"
#include "vector2.h"
#include "window.h"

namespace dukat
{
	struct CameraTransform2
	{
		Matrix4 mat_proj_orth; // orthographic projection matrix
		Matrix4 mat_view; // view matrix - not used because 
		Vector2 position; // Position in camera space
		Vector2 dimension; // Width / height of screen
		CameraTransform2(void) : position(Vector2::origin) { }
	};

	class Camera2 : public WindowEventListener
	{
	private:
		static const float default_near_clip;
		static const float default_far_clip;
		Window* window;
		float aspect_ratio;
		float near_clip;
		float far_clip;
		bool fixed_dimension;

	public:
		CameraTransform2 transform;

		// Creates a new camera for a given window. Unless specified, the 
		// camera will use the dimensions of the window.
		Camera2(Window* window, const Vector2& dimension = { 0.0f, 0.0f });
		virtual ~Camera2(void);

		void resize(int width, int height);
		void set_clip(float near, float far) { near_clip = near; far_clip = far; }
		void refresh(void) { resize(window->get_width(), window->get_height()); }

		// Updates the camera's view matrix. Subclasses of camera should update
		// the camera axes and call this method to update the view matrix.
		virtual void update(float delta);
	};
}
