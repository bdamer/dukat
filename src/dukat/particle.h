#pragma once

#include "vector2.h"
#include "color.h"

namespace dukat
{
	struct Particle
	{
		Vector2 pos;	// position in world space
		float ry;		// Axis of reflection
		Color color;	// particle color
		float size;		// size
		Vector2 dp;		// Change in position / velocity per second
		Color dc;		// Change in color / transparency per second
		float dsize;	// Change in size per second
		float ttl;		// time-to-live
		bool rendered;	// true if this particle was rendered during last frame

		Particle() : pos(), ry(0.0f), color(), size(1), dp(), dc(), dsize(0), ttl(0.0f), rendered(false) { }	
	};
}