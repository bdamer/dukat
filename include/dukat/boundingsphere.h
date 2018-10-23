#pragma once

#include "boundingbody3.h"
#include "vector3.h"

namespace dukat
{
    class BoundingSphere : public BoundingBody3
    {
    public:
        Vector3 center;
        float radius;

        BoundingSphere(const Vector3& center, float radius) : center(center), radius(radius) { }
        ~BoundingSphere(void) { }

        // Transforms bounding body based on matrix.
        std::unique_ptr<BoundingBody3> transform(const Matrix4& m);        
        // Containment / Intersection tests
        bool contains(const Vector3& p) const { return (p - center).mag() <= radius; }
        // Checks if body intersects sphere.
        bool intersect_sphere(const Vector3& center, float radius) const;
        // Checkfs if body intersects ray. Will return the distance along intersection ray or no_intersection.
        float intersect_ray(const Ray3& ray, float near, float far, Vector3* return_normal = nullptr) const;
    };
}
