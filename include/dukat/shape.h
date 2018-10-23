#pragma once

#include "vector2.h"

namespace dukat
{
    // Abstract base class representing a 2D shape in a normalized space [-1..1].
    class Shape
    {
    public:
        Shape(void) { }
        virtual ~Shape(void) { }

        // Returns true if a point is "inside" the shape.
        virtual bool contains(const Vector2& p) const = 0;
    };

    // Simple circle shape.
    class CircleShape : public Shape
    {
    private:
        const float radius;

    public:
        CircleShape(float radius) : radius(radius) { }
        ~CircleShape(void) { }

        bool contains(const Vector2& p) const { return p.mag() <= radius; }
    };
}