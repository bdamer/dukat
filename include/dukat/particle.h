#pragma once

#include "vector2.h"
#include "color.h"

namespace dukat
{
	struct Particle
	{
		enum Flags
		{
			Alive = 1,			// True if this particle is in use
			Rendered = 2,		// True if this particle was on screen during last frame
			Linear = 4,			// Update position based on dp
			Dampened = 8,		// Update dp based on dampening constant until it reaches 0.
			Gravitational = 16,	// Update dp based on gravity constant
		};

		Vector2 pos;	// Position in world space
		Vector2 dp;		// Change in position / velocity per second
		Color color;	// Particle color
		Color dc;		// Change in color / transparency per second
		float size;		// Size
		float dsize;	// Change in size per second
		float ttl;		// Time-to-live
		float ry;		// Axis of reflection
		uint8_t	flags;	// Flags

		Particle() : pos(), ry(0.0f), color(), size(1.0f), dp(), dc(), dsize(0.0f), ttl(0.0f), flags(0x4) { }	
	};
}