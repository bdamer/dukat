#include "stdafx.h"
#include <dukat/mathutil.h>
#include <dukat/particleemitter.h>
#include <dukat/particle.h>
#include <dukat/particlemanager.h>
#include <dukat/renderlayer2.h>

namespace dukat
{
	const ParticleEmitter::Recipe ParticleEmitter::Recipe::FlameRecipe{
		ParticleEmitter::Recipe::Type::Flame, 1.0f, 6.0f, 25.0f, 40.0f, 1.0f, 5.0f,
		{ 
			Color{ 1.0f, 1.0f, 0.0f, 1.0f },	// Center color
			Color{ 0.0f, 0.25f, 0.0f, 0.0f },	// Color reduction as we move away from center
			Color{ 0.0f, -0.5f, 0.0f, -0.05f },	// Color reduction over time
			Color{ 0.0f, 0.0f, 0.0f, 0.0f }		// Not used
		}
	};

	const ParticleEmitter::Recipe ParticleEmitter::Recipe::SmokeRecipe{
		ParticleEmitter::Recipe::Type::Smoke, 4.f, 8.f, 15.f, 25.f, 2.f, 6.f,
		{
			Color{ 1.0f, 1.0f, 1.0f, 1.0f },	// Smoke color
			Color{ 0.0f, 0.0f, 0.0f, -0.5f },	// Color reduction over time
			Color{ 0.0f, 0.0f, 0.0f, 0.0f },	// Not used
			Color{ 0.0f, 0.0f, 0.0f, 0.0f }		// Not used
		}
	};

	const ParticleEmitter::Recipe ParticleEmitter::Recipe::FountainRecipe{
		ParticleEmitter::Recipe::Type::Fountain, 1.f, 4.f, 40.f, 50.f, 4.f, 6.f,
		{
			Color{ 0.47f, 0.945f, 1.0f, 0.8f }, // Each color picked randomly
			Color{ 0.0f, 0.8f, 0.976f, 0.866f },
			Color{ 0.0f, 0.596f, 0.862f, 0.933f },
			Color{ 0.0f, 0.411f, 0.666f, 1.0f }
		}
	};

	const ParticleEmitter::Recipe ParticleEmitter::Recipe::ExplosionRecipe{
		ParticleEmitter::Recipe::Type::Explosion, 1.f, 6.f, 25.f, 35.f, 1.f, 5.f,
		{
			Color{ 1.0f, 1.0f, 0.0f, 1.0f },	// Initial color
			Color{ 0.0f, -0.5f, 0.0f, -0.1f },	// Color reduction over time
			Color{ 0.0f, 0.0f, 0.0f, 0.0f },	// Not used
			Color{ 0.0f, 0.0f, 0.0f, 0.0f }		// Not used
		}
	};

    void FlameEmitter::update(ParticleManager* pm, float delta)
    {
        angle += randf(-max_change, max_change);
        accumulator += rate * delta;

        if (accumulator < 1.0f || target_layer == nullptr)
            return;

        while (accumulator >= 1.0f)
        {
			auto p = pm->create_particle();
			
			const auto offset = Vector2{ 0.0f, -range }.rotate(angle);
            p->pos = pos + Vector2{ offset.x, 0.0f };
            p->dp.x = 2.0f * offset.x;
            p->dp.y = -randf(recipe.min_speed, recipe.max_speed);

            auto n_size = randf(0.0f, 1.0f);
            p->size = recipe.min_size + n_size * (recipe.max_size - recipe.min_size);

            // determine initial color of particle based on distance from center 
			// TODO: revise this - idea is that for offsets that are further from pos.x, go into red
            auto dist = std::abs(p->pos.x) / 8.0f;
			p->color = recipe.colors[0] - recipe.colors[1] * dist;
			p->dc = recipe.colors[2];
			p->dc.a -= n_size;

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
			auto p = pm->create_particle();
			
			const auto offset = Vector2{ 0.0f, -range }.rotate(angle);
            p->pos = pos + Vector2{ offset.x, 0.0f };
            p->dp.x = offset.x;
            p->dp.y = -randf(recipe.min_speed, recipe.max_speed);

			const auto size = randf(0.0f, 1.0f);
            p->size = recipe.min_size + size * (recipe.max_size - recipe.min_size);
            p->color = recipe.colors[0];
			p->dc = recipe.colors[1] * (0.1f + size); 
			
            // The smaller the particle, the longer it will live
            p->ttl = recipe.min_ttl + (1.f - size) * (recipe.max_ttl - recipe.min_ttl);

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
			auto p = pm->create_particle();
			p->flags |= Particle::Gravitational;

			const auto offset = Vector2{ 0.0f, -range }.rotate(angle);
            p->pos = pos + Vector2{ offset.x, 0.0f };
            p->dp.x = offset.x;
            p->dp.y = -randf(recipe.min_speed, recipe.max_speed);
            
            auto size = randi(1, recipe.colors.size());
            p->size = randf(recipe.min_size, recipe.max_size);
            p->color = recipe.colors[size - 1];
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

		static const Vector2 base_vector{ 0.0f, -1.0f };

		for (int i = 0; i < static_cast<int>(rate); i++)
		{
			auto p = pm->create_particle();
			p->flags |= Particle::Dampened;

			const auto angle = randf(0.0f, two_pi);
			const auto offset = base_vector.rotate(angle);
			p->pos = pos;
			p->dp = offset * randf(recipe.min_speed, recipe.max_speed);

			const auto n_size = randf(0.0f, 1.0f);
			p->size = recipe.min_size + n_size * (recipe.max_size - recipe.min_size);

			p->color = recipe.colors[0];
			p->dc = recipe.colors[1];
			p->dc.a -= n_size;

			// The smaller the particle, the longer it will live
			p->ttl = recipe.min_ttl + (1.f - n_size) * (recipe.max_ttl - recipe.min_ttl);

			target_layer->add(p);
		}
    }
}