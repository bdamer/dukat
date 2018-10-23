#pragma once

#include "mathutil.h"
#include "shape.h"

namespace dukat
{
    class IslandShape : public Shape
    {
    private:
        const float island_factor; // 1.0 means no small islands; 2.0 leads to a lot
        const int bumps;
        const float start_angle;
        const float dip_angle;
        const float dip_width;

    public:
        IslandShape(void) : island_factor(1.07f), bumps(1 + (std::rand() % 6)),
          start_angle(randf(0.0f, two_pi)), dip_angle(randf(0.0f, two_pi)), dip_width(randf(0.2f, 0.7f)) { }

        bool contains(const Vector2& q) const
        {
            auto angle = (float)std::atan2(q.x, q.y);
            auto length = 0.5f * (std::max(std::abs(q.x), std::abs(q.y)) + q.mag());
            
            auto r1 = 0.5f + 0.4f * std::sin(start_angle + bumps * angle + std::cos((bumps + 3) * angle));
            auto r2 = 0.7f - 0.2f * std::sin(start_angle + bumps * angle - std::sin((bumps + 2)*angle));
            if (std::abs(angle - dip_angle) < dip_width
                || std::abs(angle - dip_angle + two_pi) < dip_width
                || std::abs(angle - dip_angle - two_pi) < dip_width) {
                r1 = r2 = 0.2f;
            }
            return  (length < r1 || (length > r1 * island_factor && length < r2));
        }
    };
}