#pragma once

#include <memory>

namespace dukat
{
    class Ray3;
    class Vector3;
    class Matrix4;

    class BoundingBody3
    {
    public:
        BoundingBody3(void) { }
        virtual ~BoundingBody3(void) { }

        // Transforms bounding body based on matrix.
        virtual std::unique_ptr<BoundingBody3> transform(const Matrix4& matrix) = 0;
        // Containment / Intersection tests
        virtual bool contains(const Vector3& p) const = 0;
        // Checks if body intersects sphere.
        virtual bool intersect_sphere(const Vector3& center, float radius) const = 0;
        // Checkfs if body intersects ray. Will return the distance along intersection ray or no_intersection.
        virtual float intersect_ray(const Ray3& ray, float near, float far, Vector3* return_normal = nullptr) const = 0;
    };
}
