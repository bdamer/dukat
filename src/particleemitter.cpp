#include "stdafx.h"
#include <dukat/mathutil.h>
#include <dukat/particleemitter.h>
#include <dukat/particle.h>
#include <dukat/particlemanager.h>
#include <dukat/renderlayer2.h>

namespace dukat
{
    constexpr std::array<Color, 5> FountainEmitter::colors;

    void FireEmitter::update(ParticleManager* pm, float delta)
    {
        angle += randf(-max_change, max_change);
        accumulator += rate * delta;

        if (accumulator < 1.0f || target_layer == nullptr)
            return;

        while (accumulator >= 1.0f)
        {
            const auto offset = Vector2{ 0.0f, -range }.rotate(angle);

            auto p = pm->create_particle();
            p->pos = pos + Vector2{ offset.x, 0.0f };
            p->dp.x = 2.0f * offset.x;
            p->dp.y = -randf(recipe.min_speed, recipe.max_speed);

            auto n_size = randf(0.0f, 1.0f);
            p->size = recipe.min_size + n_size * (recipe.max_size - recipe.min_size);

            // the further from center the more red particle starts out as
            auto dist = std::abs(p->pos.x) / 8.0f;
            p->color = { 1.0f, 1.0f - dist * 0.25f, 0.0f, 1.0f };
            p->dc = { 0.0f, -0.5f, 0.0f, -0.05f - n_size * 1.0f };

            // The smaller the particle, the longer it will live
            p->ttl = recipe.min_ttl + (1.f - n_size) * (recipe.max_ttl - recipe.min_ttl);

            target_layer->add(p);

            accumulator -= 1.0f;
        }
    }

    void SmokeEmitter::update(ParticleManager* pm, float delta)
    {
        angle += randf(-max_change, max_change);
        accumulator += rate * delta;

        if (accumulator < 1.0f || target_layer == nullptr)
            return;

        while (accumulator >= 1.0f)
        {
            const auto offset = Vector2{ 0.0f, -range }.rotate(angle);

            auto p = pm->create_particle();
            p->pos = pos + Vector2{ offset.x, 0.0f };
            p->dp.x = offset.x;
            p->dp.y = -randf(recipe.min_speed, recipe.max_speed);

            auto n_size = randf(0.0f, 1.0f);
            p->size = recipe.min_size + n_size * (recipe.max_size - recipe.min_size);

            p->color = { 1.0f, 1.0f, 1.0f, 1.0f };
            p->dc = { 0.0f, 0.0f, 0.0f, -0.1f - n_size * 0.5f };

            // The smaller the particle, the longer it will live
            p->ttl = recipe.min_ttl + (1.f - n_size) * (recipe.max_ttl - recipe.min_ttl);

            target_layer->add(p);

            accumulator -= 1.0f;
        }
    }

    void FountainEmitter::update(ParticleManager* pm, float delta)
    {
        angle += randf(-max_change, max_change);
        accumulator += rate * delta;
        time += delta;

        if (accumulator < 1.0f || target_layer == nullptr)
            return;

        const auto max_v = recipe.max_speed * (1.0f + 0.25f * fast_cos(time));

        while (accumulator >= 1.0f)
        {
            const auto offset = Vector2{ 0.0f, -range }.rotate(angle);

            auto p = pm->create_particle();
            p->flags |= Particle::Gravitational;
            p->pos = pos + Vector2{ offset.x, 0.0f };
            p->dp.x = offset.x;
            p->dp.y = -randf(recipe.min_speed, recipe.max_speed);
            
            auto size = randi(1, 6);
            p->size = randf(recipe.min_size, recipe.max_size);
            p->color = colors[size - 1];
            p->dc = { 0.0f, 0.0f, 0.0f, -0.05f };
            p->ttl = randf(recipe.min_ttl, recipe.max_ttl);

            target_layer->add(p);

            accumulator -= 1.0f;
        }
    }

    void ExplosionEmitter::update(ParticleManager* pm, float delta)
    {
        time -= delta;
		if (time > 0.0f)
            return;

        if (repeat_interval >= 0.0f)
            time = repeat_interval;
        
		for (int i = 0; i < static_cast<int>(rate); i++)
		{
			const auto angle = randf(0.0f, two_pi);
			const auto offset = Vector2{ 0.0f, -1.0f }.rotate(angle);

			auto p = pm->create_particle();
			p->flags |= Particle::Dampened;
			p->pos = pos;
			p->dp = offset * randf(recipe.min_speed, recipe.max_speed);

			const auto n_size = randf(0.0f, 1.0f);
			p->size = recipe.min_size + n_size * (recipe.max_size - recipe.min_size);

			// the further from center the more red a particle starts out as
			auto dist = std::abs(p->pos.x) / 8.0f;
			p->color = { 1.0f, 1.0f - dist * 0.25f, 0.0f, 1.0f };
			p->dc = { 0.0f, -0.5f, 0.0f, -0.1f - n_size * 0.8f };

			// The smaller the particle, the longer it will live
			p->ttl = recipe.min_ttl + (1.f - n_size) * (recipe.max_ttl - recipe.min_ttl);

			target_layer->add(p);
		}
    }
}