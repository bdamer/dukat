#include "stdafx.h"
#include "boundingsphere.h"
#include "mathutil.h"
#include "matrix4.h"
#include "ray3.h"

namespace dukat
{
    std::unique_ptr<BoundingBody3> BoundingSphere::transform(const Matrix4& matrix)
    {
        return std::make_unique<BoundingSphere>(center * matrix, radius);
    }

    bool BoundingSphere::intersect_sphere(const Vector3& center, float radius) const
    {
        float dist = (center - this->center).mag();
        return dist > (radius + this->radius);
    }

    float BoundingSphere::intersect_ray(const Ray3& ray, float near, float far, Vector3* return_normal) const
    {
        float a = ray.dir.mag2();
        float b = 2.0f * (ray.dir.x * (ray.origin.x - center.x) + ray.dir.y * (ray.origin.y - center.y) + ray.dir.z * (ray.origin.z - center.z));
        float c = center.mag2() + ray.origin.mag2() - 
            2.0f * (center.x * ray.origin.x + center.y * ray.origin.y + center.z * ray.origin.z) - 
            radius * radius;

        float d = b * b - 4 * a * c;
        if (d < 0.0f)
        {
            return no_intersection;
        }

        // return point of intersection along ray.
        return (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
    }
}
