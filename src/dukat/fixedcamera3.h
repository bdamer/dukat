#pragma once

#include "camera3.h"
#include "vector3.h"

namespace dukat
{
    class FixedCamera3 : public Camera3
    {
    private:

    public:
        Vector3 look_at;

        FixedCamera3(Window* window, const Vector3& eye, const Vector3& look_at, const Vector3& up);
        ~FixedCamera3(void) { }

        void update(float delta);
        // Moves the eye and look-at position of the camera.
        void move(const dukat::Vector3& v) { transform.position += v; look_at += v; }
        // Changes distance from look-at position along direction vector.
        float get_distance(void) const { return (look_at - transform.position).mag(); }
        void set_distance(float distance) { transform.position = look_at - transform.dir * distance; }
    };
}