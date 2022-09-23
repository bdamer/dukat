#pragma once

#include "aabb3.h"
#include "matrix4.h"
#include "plane.h"
#include "recipient.h"
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

		CameraTransform3(void) : position(Vector3::origin),
			dir(-Vector3::unit_z), up(Vector3::unit_y), right(-Vector3::unit_x) { }
	};

	class Camera3
	{
	private:
		// Default values
		static const float default_fov;
		static const float default_near_clip;
		static const float default_far_clip;
		Window* window;
		// Horizontal and vertical field of view in degrees
		float fov_h, fov_v;
		float aspect_ratio;
		float near_clip;
		float far_clip;

		// Clip planes
		Plane left_clip_plane, right_clip_plane;

		void compute_horizontal_fov(void);

	public:
		CameraTransform3 transform;
		Camera3(Window* window);
		virtual ~Camera3(void);

		void resize(int width, int height);
		void set_vertical_fov(float fov) { fov_v = fov; compute_horizontal_fov(); }
		float get_vertical_fov(void) const { return fov_v; }
		float get_horizontal_fov(void) const { return fov_h; }
		float get_aspect_ratio(void) const { return aspect_ratio; }
		void set_clip(float near, float far) { near_clip = near; far_clip = far; }
		const Plane& get_left_clip_plane(void) const { return left_clip_plane; }
		const Plane& get_right_clip_plane(void) const { return right_clip_plane; }
		float get_near_clip(void) const { return near_clip; }
		float get_far_clip(void) const { return far_clip; }
		void refresh(void) { resize(window->get_width(), window->get_height()); }

		// Returns true if a AABB3 is not visible to this camera.
		inline bool is_clipped(const AABB3& bb) const { return (bb.classify_plane(left_clip_plane) < 0 || bb.classify_plane(right_clip_plane) < 0); }

		// Updates the camera's view matrix. Subclasses of camera should update
		// the camera axes and call this method to update the view matrix.
		virtual void update(float delta);

		// Gets and sets distance between camera position and focal point.
		virtual float get_distance(void) const = 0;
		virtual void set_distance(float distance) = 0;

		// Computes a pick ray for a set of coordinates in screen space.
		Ray3 pick_ray_screen(int x, int y);
		// Computes a pick ray for a set of coordinates in view space.
		Ray3 pick_ray_view(float x, float y);
		// Computes left and right clip planes.
		static void compute_clip_planes(const CameraTransform3& transform, float fov, Plane& left_plane, Plane& right_plane);
	};
}