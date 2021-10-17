#pragma once

#include "light.h"
#include "mathutil.h"
#include "vector3.h"

namespace dukat
{
	// TODO: revise and make general purpose animation for lights
    struct OrbitalLight
    {
        float period; // time of 1 rotation
        float theta; // angle of rotation

        OrbitalLight(float period) : period(period), theta(0.0f) { }

        void update(float delta, Light3& light)
        {
			// TODO: currently only working for directional light
            theta += delta * two_pi / period;
            light.position.x = light.position.y = -std::cos(theta);
            light.position.z = -std::sin(theta);
            light.position.normalize();
        }
    };
}