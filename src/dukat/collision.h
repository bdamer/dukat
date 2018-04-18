#pragma once

#include "vector2.h"

namespace dukat
{
	struct Collision
	{
		Vector2 pos;
		Vector2 delta;
		Vector2 normal;
	};
}