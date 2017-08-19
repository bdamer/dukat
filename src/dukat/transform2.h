#pragma once

#include "vector2.h"

namespace dukat
{
	struct Transform2
	{
		// X/Y Coordinates
		Vector2 pos;
		// Rotation in radians.
		float rot;
		// Rendering priority. Higher values will render above lower ones.
		float priority;

		// Constructor
		Transform2(void) : pos(Vector2(0,0)), rot(0), priority(-1.0f) { }
		Transform2(float x, float y, float rot) : pos(x,y), rot(rot) { }
		Transform2(Vector2 pos, float rot) : pos(pos), rot(rot) { }

		// Operators
		Transform2 operator+(const Transform2& a) const { return Transform2(pos.x + a.pos.x, pos.y + a.pos.y, rot + a.rot); }
		Transform2 operator-(const Transform2& a) const { return Transform2(pos.x - a.pos.x, pos.y - a.pos.y, rot - a.rot); }
	};
}