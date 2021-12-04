#pragma once

#include "camera3.h"
#include "mathutil.h"

namespace dukat
{
	class GameBase;

	// 3D camera which orbits around look-at point at a fixed distance.
	class OrbitCamera3 : public Camera3 
	{
	private:
		float distance; // distance from eye to target
		float min_distance, max_distance; // distance limits
		float longitude; // 0 to 2 pi
		float latitude; // from 0 (equator) to pi/2
		float rot_speed;
		GameBase* game;
		Vector3 look_at;

	public:
		OrbitCamera3(GameBase* game, const Vector3& target, float distance, float longitude, float latitude);
		~OrbitCamera3(void) { }

		void update(float delta);
		// Moves the eye and look-at position of the camera.
		void move(const Vector3& offset) { look_at += offset; }
		// Updates the eye and look-at position of the camera to a new point.
		void set_look_at(const Vector3& look_at) { this->look_at = look_at; }
		const Vector3& get_look_at(void) const { return look_at; }
		
		// Changes distance from look-at position along direction vector.
		float get_distance(void) const { return distance; }
		void set_distance(float distance) { this->distance = distance; clamp(this->distance, min_distance, max_distance); }
		float get_max_distance(void) const { return max_distance; }
		void set_max_distance(float max_distance) { this->max_distance = max_distance; }
		float get_min_distance(void) const { return max_distance; }
		void set_min_distance(float min_distance) { this->min_distance = min_distance; }
		float get_latitude(void) const { return latitude; }
		void set_latitude(float latitude) { this->latitude = latitude; }
		float get_longitude(void) const { return longitude; }
		void set_longitude(float longitude) { this->longitude = longitude; }
		void set_rot_speed(float speed) { rot_speed = speed; }
	};
}