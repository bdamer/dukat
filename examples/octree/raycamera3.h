#pragma once

#include <dukat/camera3.h>

namespace dukat
{
    class RayCamera3 : public Camera3
    {
    public:
		RayCamera3(Window* window) : Camera3(window) { }
		~RayCamera3(void) { }

        // TODO: implement if needed
        float get_distance(void) const { return 0.0f; }
        void set_distance(float distance) { };
    };
}