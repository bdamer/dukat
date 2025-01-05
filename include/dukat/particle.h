#pragma once

#include "vector2.h"
#include "color.h"
#include "memorypool.h"

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
			AntiGravitational = 32, // Update dp based on negative gravity constant
			Spiraling = 64,		// Update dp based on angular velocity
		};

		Vector2 pos;	// Position in world space
		Vector2 dp;		// Change in position / velocity per second for linear motion
						// or 
						// [angular, vertical] velocity for spiraling motion
		Vector2 ref;	// Axis of reflection
		Color color;	// Particle color
		Color dc;		// Change in color / transparency per second
		float size;		// Size
		float dsize;	// Change in size per second
		float ttl;		// Time-to-live
		float radius;	// Radius of the spiral
		float angle;	// Current angle for spiraling motion
		uint8_t	flags;	// Flags

		Particle(void) : pos(), ref(), color(), size(1.f), dp(), dc(), 
			dsize(0.f), ttl(0.f), radius(0.f), angle(0.f), 
			flags(Particle::Alive | Particle::Linear) { }

		// Custom memory allocation
		static MemoryPool<Particle> _pool;
		static void* operator new(std::size_t size) { return _pool.allocate(size); }
		static void operator delete(void* ptr, std::size_t size) { return _pool.free(ptr, size); }
	};
}