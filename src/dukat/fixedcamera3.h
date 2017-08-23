#pragma once

#include "camera3.h"
#include "gamebase.h"
#include "vector3.h"

namespace dukat
{
    class FixedCamera3 : public Camera3
    {
    private:

    public:
        Vector3 look_at;

        FixedCamera3(GameBase* game, const Vector3& eye, const Vector3& look_at, const Vector3& up);
        ~FixedCamera3(void) { }

        void update(float delta);

        // Moves the eye and look-at position of the camera.
        void move(const Vector3& offset) { transform.position += offset; look_at += offset; }
        // Updates the look-at position of the camera to a new point and updates the eye position of 
        // the camera accordingly.
		void set_look_at(const Vector3& look_at);
        // Changes distance from look-at position along direction vector.
        float get_distance(void) const { return (look_at - transform.position).mag(); }
        void set_distance(float distance) { transform.position = look_at - transform.dir * distance; }
    };
}