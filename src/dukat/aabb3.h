#pragma once

#include <vector>
#include "vector3.h"
#include "boundingbody3.h"

namespace dukat
{
	class AABB3;
	class Matrix4;
	class Ray3;

	// TODO: move to Oriented Bounding Boxes at some point, so we don't have to recompute
	// each step for animated objects
	class AABB3 : public BoundingBody3
	{
	public:
		Vector3 min;
		Vector3 max;

        AABB3(void) { clear(); }
		AABB3(const std::vector<Vector3>& vectors);
	
		// Dimension queries
		Vector3 size() const { return max - min; }
		float x_size() { return max.x - min.x; }
		float y_size() { return max.y - min.y; }
		float z_size() { return max.z - min.z; }
		Vector3 center() const { return (min + max) * 0.5f; }
		Vector3 corner(int i) const;

		// Box operations
		void clear(void);
		void add(const Vector3& p);
		void add(const AABB3& box);
	
		// Transform the box and compute the new AABB
		void set_to_transformed_box(const AABB3& box, const Matrix4& m);
        // Transforms bounding body based on matrix.
        std::unique_ptr<BoundingBody3> transform(const Matrix4& m);

        // Containment / Intersection tests
		bool empty(void) const;
		bool contains(const Vector3& p) const;
		// Checks if AABB intersects sphere.
		bool intersect_sphere(const Vector3& center, float radius) const;
        // Checkfs if body intersects ray. Will return the distance along intersection ray or no_intersection.
        float intersect_ray(const Ray3& ray, float near, float far, Vector3* return_normal = nullptr) const;
		int classify_plane(const Vector3& n, float d) const;
		float intersect_plane(const Vector3& n, float planeD, const Vector3& dir) const;
		bool intersect_aabb(const AABB3& box1, const AABB3& box2, AABB3* box_intersect = 0);
		float intersect_moving_aabb(const AABB3& stationary_box, const AABB3& moving_box, const Vector3& d);
	};
}