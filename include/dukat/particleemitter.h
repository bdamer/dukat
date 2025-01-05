#pragma once

#include <array>
#include "color.h"
#include "memorypool.h"
#include "vector2.h"
#include "particle.h"
#include "particlerecipe.h"

namespace dukat
{
    class ParticleManager;
    class RenderLayer2;

    // Abstract particle emitter base class.
    struct ParticleEmitter
    {
		// Particle recipe
        ParticleRecipe recipe;
		// Update function (called once per frame)
		std::function<void(ParticleManager& pm, ParticleEmitter& em, float delta)> update;
		// Emitter world pos
        Vector2 pos;
		// Offsets at which to emit particles
		std::vector<Vector2> offsets;
		// offset from pos to horizontal mirror axis
		float mirror_offset;
		// Layer to add particles to
        RenderLayer2* target_layer;
		// Time-to-live for the emitter
		float ttl;
		// Age of the emitter
		float age;
		// Accumulator for particles to be emitted
        float accumulator;
		// Generic value holder - can be used by update function
		float value;
		// Will only generate particles if active
		bool active;

        ParticleEmitter(void) : update(nullptr), mirror_offset(0.0f), target_layer(nullptr), ttl(0.0f), 
			age(0.0f), accumulator(0.0f), value(0.0f), active(true) { }
        ~ParticleEmitter(void) { }

		// Custom memory allocation
		static MemoryPool<ParticleEmitter> _pool;
		static void* operator new(std::size_t size) { return _pool.allocate(size); }
		static void operator delete(void* ptr, std::size_t size) { return _pool.free(ptr, size); }
    };

	// Factory method for particle emitters.
	void init_emitter(ParticleEmitter& emitter, const ParticleRecipe& recipe);
}