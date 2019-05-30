#pragma once

#include <array>
#include "color.h"
#include "vector2.h"

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
				Flame,
				Smoke,
				Fountain,
				Explosion
			};

			Type type;
			// particle size
			float min_size;
			float max_size;
			// particle speed
			float min_speed;
			float max_speed;
			// particle ttl
			float min_ttl;
			float max_ttl;
			// particle colors
			std::array<Color, 4> colors;

			Recipe(void) : min_size(0.0f), max_size(0.0f), min_speed(0.0f), max_speed(0.0f), min_ttl(0.0f), max_ttl(0.0f) { }
			Recipe(Type type) : type(type), min_size(0.0f), max_size(0.0f), min_speed(0.0f), max_speed(0.0f), min_ttl(0.0f), max_ttl(0.0f) { }
			Recipe(Type type, float min_size, float max_size, float min_speed, float max_speed, float min_ttl, float max_ttl, const std::array<Color,4> colors) 
				: type(type), min_size(min_size), max_size(max_size), min_speed(min_speed), max_speed(max_speed), min_ttl(min_ttl), max_ttl(max_ttl), colors(colors) { }
			~Recipe(void) { }

			// Default recipes
			static const Recipe FlameRecipe;
			static const Recipe SmokeRecipe;
			static const Recipe FountainRecipe;
			static const Recipe ExplosionRecipe;
		};

		// Particle recipe
        Recipe recipe;
        // Emitter world pos
        Vector2 pos;
		// Offsets at which to emit particles
		std::vector<Vector2> offsets;
        // Rate of particle emission (particles / second)
        float rate;
		// offset from pos to horizontal mirror axis
		float mirror_offset;
		// Layer to add particles to
        RenderLayer2* target_layer;

		// Will only generate particles if active
		bool active;
		// Accumulator for particles to be emitted
        float accumulator;

        ParticleEmitter(void) : rate(0.0f), mirror_offset(0.0f), target_layer(nullptr), active(true), accumulator(0.0f) { }
        ParticleEmitter(const Recipe& recipe) : recipe(recipe), rate(0.0f), mirror_offset(0.0f), target_layer(nullptr), active(true), accumulator(0.0f) { }
        virtual ~ParticleEmitter(void) { }

        virtual void update(ParticleManager* pm, float delta) = 0;
    };

    struct CustomEmitter : public ParticleEmitter 
    {
        std::function<void(ParticleEmitter* em, ParticleManager* pm, float delta)> update_func;

        CustomEmitter(void) : update_func(nullptr) { }
        virtual ~CustomEmitter(void) { }

        void update(ParticleManager* pm, float delta) { if (update_func) update_func(this, pm, delta); };
    };

    // FLAME
	// - particles are emitted with upward direction
    // - flame particles have variable ttl, so that we end up with holes
    // - if using multiple emitters, each emitter should have a current 
    //   direction that swings by random amount; that will cause subsequent 
    //   particles to have similar direction
    struct FlameEmitter : public ParticleEmitter
    {
        // max change to angle per second
        float max_change;
        // current angle 
        float angle;
        // horizontal -range,range
        float range;

		FlameEmitter(void) : max_change(0.25f), angle(0.0f), range(2.0f) { }
		FlameEmitter(const Recipe& recipe) : ParticleEmitter(recipe), max_change(0.25f), angle(0.0f), range(2.0f) { }
        ~FlameEmitter(void) { }

        void update(ParticleManager* pm, float delta);
    };

    // SMOKE
    // - upward direction of particles
    // - position of emitter ocilates on the x axis
	// - single color that fades out over time
    struct SmokeEmitter : public ParticleEmitter
    {
        // max change to angle per second
        float max_change;
        // current angle 
        float angle;
        // horizontal -range,range
        float range;

        SmokeEmitter(void) : max_change(0.15f), angle(0.0f), range(4.0f) { }
        SmokeEmitter(const Recipe& recipe) : ParticleEmitter(recipe), max_change(0.15f), angle(0.0f), range(4.0f) { }
        ~SmokeEmitter(void) { }

        void update(ParticleManager* pm, float delta);
    };

    // FOUNTAIN
    // - initial dx, dy
    // - gravity pulls at dy->reduce and ultimately turn around
    struct FountainEmitter : public ParticleEmitter
    {
        // max change to angle per second
        float max_change;
        // current angle 
        float angle;
        // time accumulator
        float time;
        // horizontal -range,range
        float range;

        FountainEmitter(void) : max_change(0.2f), angle(0.0f), time(0.0f), range(4.0f) { }
        FountainEmitter(const Recipe& recipe) : ParticleEmitter(recipe), max_change(0.2f), angle(0.0f), time(0.0f), range(4.0f) { }
        ~FountainEmitter(void) { }

        void update(ParticleManager* pm, float delta);
    };

    // EXPLOSION
    // - burst of particles
    struct ExplosionEmitter : public ParticleEmitter
    {
        // repeat interval
        float repeat_interval;
        // time accumulator
        float time;

        ExplosionEmitter(void) : repeat_interval(5.0f), time(0.0f) { }
        ExplosionEmitter(const Recipe& recipe) : ParticleEmitter(recipe), repeat_interval(5.0f), time(0.0f) { } 
        ~ExplosionEmitter(void) { }

        void update(ParticleManager* pm, float delta);
    };
}