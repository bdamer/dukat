#pragma once

#include <array>
#include "color.h"
#include "memorypool.h"
#include "vector2.h"
#include "particle.h"

namespace dukat
{
    class ParticleManager;
    class RenderLayer2;

    // Abstract particle emitter base class.
    struct ParticleEmitter
    {
		struct Recipe
		{
			// Recipe for a particle emitter.
			enum Type
			{
				None,
                Linear,
				Uniform,
				Flame,
				Smoke,
				Fountain,
				Explosion,
				Spiral,
				Radial,
				Layered,
				Blast,
				GroundExplosion,
				Fog,
				Vortex
			};

			Type type;
			// Flags of particles to create.
			uint8_t flags;
			// Rate of particle emission (particles / second)
			float rate;
			// particle size
			float min_size;
			float max_size;
			// particle ttl
			float min_ttl;
			float max_ttl;
            // particle speed
            Vector2 min_dp;
            Vector2 max_dp;
			// particle colors
			std::array<Color, 4> colors;
            // color reduciton over time
            Color dc;

			Recipe(void) : type(None), flags(Particle::Alive | Particle::Linear), rate(0.0f), min_size(0.0f), max_size(0.0f), min_ttl(0.0f), max_ttl(0.0f), colors() { }
			Recipe(Type type) : type(type), flags(Particle::Alive | Particle::Linear), rate(0.0f), min_size(0.0f), max_size(0.0f), min_ttl(0.0f), max_ttl(0.0f), colors() { }
			Recipe(Type type, uint8_t flags, float rate, float min_size, float max_size, float min_ttl, float max_ttl, 
				const Vector2& min_dp, const Vector2& max_dp, const std::array<Color,4> colors, const Color& dc) 
				: type(type), flags(flags), rate(rate), min_size(min_size), max_size(max_size), min_ttl(min_ttl), max_ttl(max_ttl),
				min_dp(min_dp), max_dp(max_dp), colors(colors), dc(dc) { }
			~Recipe(void) { }
		};

		// Particle recipe
        Recipe recipe;
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

	// Default recipes
	namespace recipes
	{
		// Flame particles emitter
		// default params: rate,size,ttl,dc
		// emit distance: min_dp.x
		// horizontal motion: max_dp.x
		// vertical motion: [min_dp.y, max_dp.y]
		// colors[0]: center color
		// colors[1]: outer color
		extern const ParticleEmitter::Recipe FlameRecipe;

		// Smoke particles emitter
		// default params: rate,size,ttl,dc
		// emit distance: min_dp.x
		// horizontal motion: max_dp.x
		// vertical motion: [min_dp.y, max_dp.y]
		// colors[0]: Smoke color
		// colors[1]: R-value: rate of change to angle, other values not used
		extern const ParticleEmitter::Recipe SmokeRecipe;

		// Fog particles emitter
		// default params: rate,size,ttl,dc
		// ground particle motion range: [min_dp.x, max_dp.x]
		// escape particle motion range: [min_dp.y, max_dp.y]
		// colors[0]: Fog color
		// colors[1].rg: min/max scalar coefficient of small particles 
		// colors[1].b: Emit ratio for small particles
		extern const ParticleEmitter::Recipe FogRecipe;

		// Fountain particles emitter
		// default params: rate,size,ttl,dc
		// emit range: [min_dp.x,max_dp.x]
		// vertical motion: [min_dp.y, max_dp.y]
		// colors[0:4]: randomly selected
		extern const ParticleEmitter::Recipe FountainRecipe;

		// Explosion particle emitter
		// default params: rate,size,ttl,dc
		// emit range: [min_dp.x, max_dp.x]
		// particle motion range: [min_dp.y, max_dp.y]
		// colors[0]: inner color (at min_dp.x)
		// colors[1]: outer color (at max_dp.x)
		extern const ParticleEmitter::Recipe ExplosionRecipe;

		// Ground Explosion particle emitter
		// default params: rate,size,ttl,dc
		// emit range: [min_dp.x, max_dp.x]
		// particle motion range: [min_dp.y, max_dp.y]
		// colors[0]: inner color (at min_dp.x)
		// colors[1]: outer color (at max_dp.x)
		extern const ParticleEmitter::Recipe GroundExplosionRecipe;

		extern const ParticleEmitter::Recipe SnowRecipe;
		extern const ParticleEmitter::Recipe SpiralRecipe;
		extern const ParticleEmitter::Recipe BlastRecipe;
		extern const ParticleEmitter::Recipe VortexRecipe;
	}

	// Factory method for particle emitters.
	void init_emitter(ParticleEmitter& emitter, const ParticleEmitter::Recipe& recipe);
}