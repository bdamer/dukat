#pragma once

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
                Fire,
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

            Recipe(void) : min_size(0.0f), max_size(0.0f), min_speed(0.0f), max_speed(0.0f), min_ttl(0.0f), max_ttl(0.0f) { }
            Recipe(Type type) : type(type), min_size(0.0f), max_size(0.0f), min_speed(0.0f), max_speed(0.0f), min_ttl(0.0f), max_ttl(0.0f) { }
            ~Recipe(void) { }
        };

        // Particle recipe
        Recipe recipe;
        // Emitter world pos
        Vector2 pos;
        // Rate of particle emission (particles / second)
        float rate;
        // Layer to add particles to
        RenderLayer2* target_layer;
        // Accumulator for particles to be emitted
        float accumulator;

        ParticleEmitter(void) : rate(0.0f), target_layer(nullptr), accumulator(0.0f) { }
        ParticleEmitter(const Recipe& recipe) : recipe(recipe), rate(0.0f), target_layer(nullptr), accumulator(0.0f) { }
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

    // FIRE
	// - particles are emitted with upward direction
    // - fire particles should have variable ttl, so that we end up with holes
    // - if using multiple emitters, each emitter should have a current 
    //   direction that swings by random amount; that will cause subsequent 
    //   particles to have similar direction
    struct FireEmitter : public ParticleEmitter
    {
        // max change to angle per second
        float max_change;
        // current angle 
        float angle;
        // horizontal -range,range
        float range;

        FireEmitter(void) : max_change(0.25f), angle(0.0f), range(2.0f) { }
        FireEmitter(const Recipe& recipe) : ParticleEmitter(recipe), max_change(0.25f), angle(0.0f), range(2.0f)
        {
            if (this->recipe.min_size <= 0.0f)
                this->recipe.min_size = 1.0f;
            if (this->recipe.max_size <= 0.0f)
                this->recipe.max_size = 6.0f;
            if (this->recipe.min_ttl <= 0.0f)
                this->recipe.min_ttl = 1.0f;
            if (this->recipe.max_ttl <= 0.0f)
                this->recipe.max_ttl = 5.0f;
            if (this->recipe.min_speed <= 0.0f)
                this->recipe.min_speed = 25.0f;
            if (this->recipe.max_speed <= 0.0f)
                this->recipe.max_speed = 40.0f;
        }
        ~FireEmitter(void) { }

        void update(ParticleManager* pm, float delta);
    };

    // SMOKE
    // - upward direction of particles
    // - position of emitter ocilates on the x axis
    struct SmokeEmitter : public ParticleEmitter
    {
        // max change to angle per second
        float max_change;
        // current angle 
        float angle;
        // horizontal -range,range
        float range;

        SmokeEmitter(void) : max_change(0.15f), angle(0.0f), range(4.0f) { }
        SmokeEmitter(const Recipe& recipe) : ParticleEmitter(recipe), max_change(0.15f), angle(0.0f), range(4.0f)
        {
            if (this->recipe.min_size <= 0.0f)
                this->recipe.min_size = 4.0f;
            if (this->recipe.max_size <= 0.0f)
                this->recipe.max_size = 8.0f;
            if (this->recipe.min_ttl <= 0.0f)
                this->recipe.min_ttl = 2.0f;
            if (this->recipe.max_ttl <= 0.0f)
                this->recipe.max_ttl = 6.0f;
            if (this->recipe.min_speed <= 0.0f)
                this->recipe.min_speed = 15.0f;
            if (this->recipe.max_speed <= 0.0f)
                this->recipe.max_speed = 25.0f;
        }
        ~SmokeEmitter(void) { }

        void update(ParticleManager* pm, float delta);
    };

    // FOUNTAIN
    // - initial dx, dy
    // - gravity pulls at dy->reduce and ultimately turn around
    struct FountainEmitter : public ParticleEmitter
    {
        static constexpr std::array<Color, 5> colors = {
            Color{ 1.0f, 1.0f, 1.0f, 0.8f },
            Color{ 0.47f, 0.945f, 1.0f, 0.8f },
            Color{ 0.0f, 0.8f, 0.976f, 0.866f },
            Color{ 0.0f, 0.596f, 0.862f, 0.933f },
            Color{ 0.0f, 0.411f, 0.666f, 1.0f }
        };

        // max change to angle per second
        float max_change;
        // current angle 
        float angle;
        // time accumulator
        float time;
        // horizontal -range,range
        float range;

        FountainEmitter(void) : max_change(0.2f), angle(0.0f), time(0.0f), range(4.0f) { }
        FountainEmitter(const Recipe& recipe) : ParticleEmitter(recipe), max_change(0.2f), angle(0.0f), range(4.0f)
        {
            if (this->recipe.min_size <= 0.0f)
                this->recipe.min_size = 1.0f;
            if (this->recipe.max_size <= 0.0f)
                this->recipe.max_size = 4.0f;
            if (this->recipe.min_ttl <= 0.0f)
                this->recipe.min_ttl = 4.0f;
            if (this->recipe.max_ttl <= 0.0f)
                this->recipe.max_ttl = 4.0f;
            if (this->recipe.min_speed <= 0.0f)
                this->recipe.min_speed = 30.0f;
            if (this->recipe.max_speed <= 0.0f)
                this->recipe.max_speed = 40.0f;
        }        
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
        ExplosionEmitter(const Recipe& recipe) : ParticleEmitter(recipe), repeat_interval(5.0f), time(0.0f)
        {
            if (this->recipe.min_size <= 0.0f)
                this->recipe.min_size = 1.0f;
            if (this->recipe.max_size <= 0.0f)
                this->recipe.max_size = 6.0f;
            if (this->recipe.min_ttl <= 0.0f)
                this->recipe.min_ttl = 1.0f;
            if (this->recipe.max_ttl <= 0.0f)
                this->recipe.max_ttl = 6.0f;
            if (this->recipe.min_speed <= 0.0f)
                this->recipe.min_speed = 25.0f;
            if (this->recipe.max_speed <= 0.0f)
                this->recipe.max_speed = 35.0f;
        } 
        ~ExplosionEmitter(void) { }

        void update(ParticleManager* pm, float delta);
    };
}