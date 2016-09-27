#pragma once

#include "matrix4.h"
#include "vector3.h"
#include "window.h"

namespace dukat
{
	class Ray3;

	struct CameraTransform3 
	{
		Matrix4 mat_proj_pers; // perspective projection matrix
		Matrix4 mat_proj_orth; // orthographic projection matrix
		Matrix4 mat_view;
		Matrix4 mat_view_inv;
		Vector3 position;
		Vector3 dir;
		Vector3 up;
		Vector3 right;

		CameraTransform3::CameraTransform3(void) : position(Vector3::origin), 
			dir(-Vector3::unit_z), up(Vector3::unit_y), right(-Vector3::unit_x) { }
	};

	class Camera3 : public WindowEventListener
	{
	private:
		// Default values
		static const float default_fov;
		static const float default_near_clip;
		static const float default_far_clip;
		Window* window;
		float field_of_view;
		float aspect_ratio;
		float near_clip;
		float far_clip;

	public:
		CameraTransform3 transform;
		Camera3(Window* window);
		~Camera3(void);

		void resize(int width, int height);
		void set_fov(float fov) { field_of_view = fov; }
		float get_fov(void) { return field_of_view; }
		void set_clip(float near, float far) { near_clip = near; far_clip = far; }
		void refresh(void) { resize(window->get_width(), window->get_height()); }

		// Updates the camera's view matrix. Subclasses of camera should update
		// the camera axes and call this method to update the view matrix.
		virtual void update(float delta);

        // Gets and sets distance between camera position and focal point.
        virtual float get_distance(void) const = 0;
        virtual void set_distance(float distance) = 0;

		// Computes a pick ray for a set of coordinates in screen space.
		Ray3 pick_ray_screen(float x, float y);
		// Computes a pick ray for a set of coordinates in view space.
		Ray3 pick_ray_view(float x, float y);
	};
}