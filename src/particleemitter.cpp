#include "stdafx.h"
#include <dukat/mathutil.h>
#include <dukat/particleemitter.h>
#include <dukat/particle.h>
#include <dukat/particlemanager.h>
#include <dukat/rand.h>
#include <dukat/renderlayer2.h>

namespace dukat
{
	MemoryPool<ParticleEmitter> ParticleEmitter::_pool(512);

	namespace recipes
	{
		const ParticleEmitter::Recipe FlameRecipe{
			ParticleEmitter::Recipe::Type::Flame, 
			Particle::Alive | Particle::Linear,
			400.f, 1.f, 6.f, 1.f, 5.f,				// rate, min/max size, min/max ttl
			Vector2{ 2, 25 },	// min_dp.x used to determine initial range
			Vector2{ 2, 40 }, 	// max_dp.x used to scale particle motion 
			{ 
				Color{ 1.0f, 1.0f, 0.0f, 1.0f },	// Center color
				Color{ 0.0f, 0.25f, 0.0f, 0.0f },	// Color reduction as we move away from center
				Color{ 0.0f, 0.0f, 0.0f, 0.0f },	// Not used
				Color{ 0.0f, 0.0f, 0.0f, 0.0f }		// Not used
			},
			Color{ 0.0f, -0.5f, 0.0f, -0.05f }		// Color reduction over time
		};

		const ParticleEmitter::Recipe SmokeRecipe{
			ParticleEmitter::Recipe::Type::Smoke, 
			Particle::Alive | Particle::Linear,
			100.f, 4.f, 8.f, 2.f, 6.f,				// rate, min/max size, min/max ttl
			Vector2{ 4, 15 },	// min_dp.x used to determine initial range
			Vector2{ 1, 25 },	// max_dp.x used to scale particle motion 
			{
				Color{ 1.0f, 1.0f, 1.0f, 1.0f },	// Smoke color
				Color{ 0.15f, 0.0f, 0.0f, 0.0f },	// R-value: rate of change to angle, other values not used
				Color{ 0.0f, 0.0f, 0.0f, 0.0f },	// Not used
				Color{ 0.0f, 0.0f, 0.0f, 0.0f }		// Not used
			},
			Color{ 0.0f, 0.0f, 0.0f, -0.5f }		// Color reduction over time
		};

		const ParticleEmitter::Recipe FogRecipe{
			ParticleEmitter::Recipe::Type::Fog,
			Particle::Alive | Particle::Linear,
			40.f, 8.f, 16.f,	// rate, min/max size of ground particles
			5.f, 10.f,			// min/max ttl
			Vector2{ -2, 2 },	// _dp.x used for particle motion
			Vector2{ 2, 4 },	// _dp.y used for particle motion that escape fog bank
			{
				Color{ 1.0f, 1.0f, 1.0f, 0.85f },	// Fog color
				Color{ 0.75f, 0.75f, 0.1f, 0.0f },	// R/G - min/max scalar coefficient of small particles 
													// B - Emit ratio for small particles
				Color{ 0.0f, 0.0f, 0.0f, 0.0f },	// Not used
				Color{ 0.0f, 0.0f, 0.0f, 0.0f }		// Not used
			},
			Color{ 0.0f, 0.0f, 0.0f, 0.0f }			// Color reduction over time - alpha not used
		};

		const ParticleEmitter::Recipe FountainRecipe{
			ParticleEmitter::Recipe::Type::Fountain, 
			Particle::Alive | Particle::Linear | Particle::Gravitational,
			200.f, 1.f, 4.f, 4.f, 6.f,				// rate, min/max size, min/max ttl
			Vector2{ 0, 80 }, Vector2{ 4, 100 }, 	// x used for initial range
													// y used for initial motion in -y direction
			{
				Color{ 0.47f, 0.945f, 1.0f, 0.8f }, // Each color picked randomly
				Color{ 0.0f, 0.8f, 0.976f, 0.866f },
				Color{ 0.0f, 0.596f, 0.862f, 0.933f },
				Color{ 0.0f, 0.411f, 0.666f, 1.0f }
			},
			Color{ 0.0f, 0.0f, 0.0f, -0.1f }		// Color reduction over time
		};

		const ParticleEmitter::Recipe ExplosionRecipe{
			ParticleEmitter::Recipe::Type::Explosion, 
			Particle::Alive | Particle::Linear | Particle::Dampened,
			100.f, 1.f, 6.f, 1.f, 5.f,				// rate, min/max size, min/max ttl
			Vector2{ 0, 25 }, Vector2{ 10, 35 },	// x used to adjust initial position in particle direction 
													// y used to determine particle velocity
			{
				Color{ 1.0f, 0.78f, 0.14f, 1.0f },	// Inner color (at min_dp.x)
				Color{ 1.0f, 0.31f, 0.0f, 1.0f },	// Outer color (at max_dp.x)
				Color{ 0.0f, 0.0f, 0.0f, 0.0f },	// Not used
				Color{ 0.0f, 0.0f, 0.0f, 0.0f }		// Not used
			},
			Color{ 0.0f, -0.25f, -0.05f, -0.05f }	// Color reduction over time
		};

		const ParticleEmitter::Recipe GroundExplosionRecipe{
			ParticleEmitter::Recipe::Type::GroundExplosion,
			Particle::Alive | Particle::Linear | Particle::Gravitational,
			100.f, 1.f, 6.f, 1.f, 5.f,				// rate, min/max size, min/max ttl
			Vector2{ 0, 25 }, Vector2{ 10, 35 },	// x used to adjust initial position in particle direction 
													// y used to determine particle velocity
			{
				Color{ 1.0f, 0.78f, 0.14f, 1.0f },	// Inner color (at min_dp.x)
				Color{ 1.0f, 0.31f, 0.0f, 1.0f },	// Outer color (at max_dp.x)
				Color{ 0.0f, 0.0f, 0.0f, 0.0f },	// Not used
				Color{ 0.0f, 0.0f, 0.0f, 0.0f }		// Not used
			},
			Color{ 0.0f, -0.25f, -0.05f, -0.05f }	// Color reduction over time
		};

		const ParticleEmitter::Recipe SpiralRecipe{
			ParticleEmitter::Recipe::Type::Spiral,
			Particle::Alive | Particle::Linear,
			100.f, 1.f, 4.f, 1.f, 5.f,				// rate, min/max size, min/max ttl
			Vector2{ 4.f, 3.f },					// x used to scale emit range, y used to define max angular change per second
			Vector2{ 64, 64 },						// Used to scale dp
			{
				Color{ 1.0f, 1.0f, 0.0f, 1.0f },	// Initial color
				Color{ 0.0f, 0.0f, 0.0f, 0.0f },	// Not used
				Color{ 0.0f, 0.0f, 0.0f, 0.0f },	// Not used
				Color{ 0.0f, 0.0f, 0.0f, 0.0f }		// Not used
			},
			Color{ 0.0f, -1.0f, 0.0f, -0.1f }		// Color reduction over time
		};

		const ParticleEmitter::Recipe SnowRecipe{
			ParticleEmitter::Recipe::Layered,
			Particle::Alive | Particle::Linear,
			40.0f,	
			1.0f, 3.0f,		// size within 1,3
			10.0f, 10.0f,	// ttl 
			Vector2{ -2, 24 },  // particle motion for background (color[2] and color[3])
			Vector2{ 4, 36 },	// particle motion for foreground (color[0] and color[1])
			{
				color_rgba(0xffffffff),
				color_rgba(0xffffffc0),
				color_rgba(0xc7cfddc0),
				color_rgba(0x94fdffc0),
			},
			Color{ 0.f, 0.f, 0.f, -0.005f }
		};

		const ParticleEmitter::Recipe BlastRecipe{
			ParticleEmitter::Recipe::Blast,
			Particle::Alive | Particle::Linear | Particle::AntiGravitational,
			100.0f, // rate
			1.0f, 4.0f, // size
			1.0f, 3.0f, // ttl
			Vector2{ 32, -7 }, Vector2{ 64, 7 }, // direction
			{
				color_rgb(0xffc825),
				color_rgb(0xffa214),
				color_rgb(0xed7614),
				color_rgb(0xff5000)
			},
			Color{ 0.1f, -0.1f, -0.1f, -0.25f }
		};
	}

	// LINEAR
	// - particles are created with unique direction in +/- dp range
	void linear_update(ParticleManager& pm, ParticleEmitter& em, float delta)
	{
        em.accumulator += em.recipe.rate * delta;
        if (em.accumulator < 1.0f || em.target_layer == nullptr)
            return;

		const auto offset_count = em.offsets.size();
        while (em.accumulator >= 1.0f)
        {
			auto p = pm.create_particle();
			if (p == nullptr)
				return;
			p->flags = em.recipe.flags;
			p->ry = em.pos.y + em.mirror_offset;
			
			if (offset_count == 0)
				p->pos = em.pos;
			else
				p->pos = em.pos + em.offsets[random(0, offset_count)];

			p->dp = random(em.recipe.min_dp, em.recipe.max_dp);
            p->size = random(em.recipe.min_size, em.recipe.max_size);
			p->color = em.recipe.colors[random(0, em.recipe.colors.size())];
			p->dc = em.recipe.dc;
            p->ttl = random(em.recipe.min_ttl, em.recipe.max_ttl);
			
			em.target_layer->add(p);

			em.accumulator -= 1.0f;
        }
	}

	// UNIFORM
	// - particles are generated within a box based on offset[0] - offset[1]
	// - particles are created with fixed direction within +/- dp range
	void uniform_update(ParticleManager& pm, ParticleEmitter& em, float delta)
	{
		em.accumulator += em.recipe.rate * delta;
		if (em.accumulator < 1.0f || em.target_layer == nullptr)
			return;
		if (em.offsets.size() < 2)
			return; 

		const auto& min_offset = em.offsets[0];
		const auto& max_offset = em.offsets[1];
		while (em.accumulator >= 1.0f)
		{
			auto p = pm.create_particle();
			if (p == nullptr)
				return;
			p->flags = em.recipe.flags;
			p->ry = em.pos.y + em.mirror_offset;
			p->pos = em.pos + random(min_offset, max_offset);

			p->dp = random(em.recipe.min_dp, em.recipe.max_dp);
			p->size = random(em.recipe.min_size, em.recipe.max_size);
			p->color = em.recipe.colors[random(0, em.recipe.colors.size())];
			p->dc = em.recipe.dc;
			p->ttl = random(em.recipe.min_ttl, em.recipe.max_ttl);

			em.target_layer->add(p);

			em.accumulator -= 1.0f;
		}
	}

	// LAYERED
	// - particles are generated within a box based on offset[0] - offset[1]
	// - background particles are created with min_dp and colors 2 and 3
	// - foreground particles are created with max_dp and colors 0 and 1
	void layered_update(ParticleManager& pm, ParticleEmitter& em, float delta)
	{
		em.accumulator += em.recipe.rate * delta;
		if (em.accumulator < 1.0f || em.target_layer == nullptr)
			return;

		if (em.offsets.size() < 2)
			return;

		const auto& min_offset = em.offsets[0];
		const auto& max_offset = em.offsets[1];
		while (em.accumulator >= 1.0f)
		{
			auto p = pm.create_particle();
			if (p == nullptr)
				return;
			p->flags = em.recipe.flags;
			p->ry = em.pos.y + em.mirror_offset;
			p->pos = em.pos + random(min_offset, max_offset);

			const auto z = random(0, em.recipe.colors.size());
			p->dp = (z >= 2) ? em.recipe.min_dp : em.recipe.max_dp;

			p->size = static_cast<float>(random(static_cast<int>(em.recipe.min_size), static_cast<int>(em.recipe.max_size)));
			
			p->color = em.recipe.colors[z];
			p->dc = em.recipe.dc;
			p->ttl = random(em.recipe.min_ttl, em.recipe.max_ttl);

			em.target_layer->add(p);

			em.accumulator -= 1.0f;
		}
	}

	// RADIAL
	// - continuous particles generated at a radius around emitter
	// - particle direction determined by random angle
	// - particle position based min_dp.x along direction vector
	// - particle velocity within min_dp.y and max_dp.y along direction
	void radial_update(ParticleManager& pm, ParticleEmitter& em, float delta)
	{
		em.accumulator += em.recipe.rate * delta;
		if (em.accumulator < 1.0f || em.target_layer == nullptr)
			return;

		static const Vector2 base_vector{ 0.0f, -1.0f };
		const auto offset_count = em.offsets.size();
		while (em.accumulator >= 1.0f)
		{
			auto p = pm.create_particle();
			if (p == nullptr)
				return;
			p->flags = em.recipe.flags;
			p->ry = em.pos.y + em.mirror_offset;

			const auto angle = random(0.0f, two_pi);
			const auto offset = base_vector.rotate(angle);
			if (offset_count == 0)
				p->pos = em.pos;
			else
				p->pos = em.pos + em.offsets[random(0, offset_count)];

			if (em.recipe.min_dp.x != 0.0f)
				p->pos += offset * em.recipe.min_dp.x;

			p->dp = offset * random(em.recipe.min_dp.y, em.recipe.max_dp.y);
			p->size = random(em.recipe.min_size, em.recipe.max_size);
			p->color = em.recipe.colors[random(0, em.recipe.colors.size())];
			p->dc = em.recipe.dc;
			p->ttl = random(em.recipe.min_ttl, em.recipe.max_ttl);

			em.target_layer->add(p);

			em.accumulator -= 1.0f;
		}
	}

	// FLAME
	// - particles are emitted with upward direction
	// - flame particles have variable ttl, so that we end up with holes
	// - if using multiple emitters, each emitter should have a current 
	//   direction that swings by random amount; that will cause subsequent 
	//   particles to have similar direction
	void flame_update(ParticleManager& pm, ParticleEmitter& em, float delta)
    {
		const auto max_change = 0.25f;
        em.value += random(-max_change, max_change);
		em.accumulator += em.recipe.rate * delta;

        if (em.accumulator < 1.0f || em.target_layer == nullptr)
            return;

		const auto range = em.recipe.min_dp.x;
		const auto offset_count = em.offsets.size();
        while (em.accumulator >= 1.0f)
        {
			auto p = pm.create_particle();
			if (p == nullptr)
				break;

			p->flags = em.recipe.flags;
			p->ry = em.pos.y + em.mirror_offset;
			
			auto offset = Vector2{ 0.f, range }.rotate(em.value);
			offset.y = 0.f;

			p->dp.x = em.recipe.max_dp.x * offset.x;
			p->dp.y = -random(em.recipe.min_dp.y, em.recipe.max_dp.y);

			if (offset_count > 0)
				offset += em.offsets[random(0, offset_count)];
			p->pos = em.pos + offset;

            auto n_size = random(0.0f, 1.0f);
            p->size = em.recipe.min_size + n_size * (em.recipe.max_size - em.recipe.min_size);

            // determine initial color of particle based on distance from center 
			// TODO: revise this - idea is that for offsets that are further from pos.x, go into red
            auto dist = std::abs(offset.x) / (4.0f * range);
			p->color = em.recipe.colors[0] - em.recipe.colors[1] * dist;
			p->dc = em.recipe.dc;
			p->dc.a -= n_size;

            // The smaller the particle, the longer it will live
            p->ttl = em.recipe.min_ttl + (1.f - n_size) * (em.recipe.max_ttl - em.recipe.min_ttl);

			em.target_layer->add(p);

			em.accumulator -= 1.0f;
        }
    }

	// SMOKE
	// - upward direction of particles
	// - position of emitter oscillates on the x axis
	// - single color that fades out over time
	void smoke_update(ParticleManager& pm, ParticleEmitter& em, float delta)
    {
		em.value += random(-em.recipe.colors[1].r, em.recipe.colors[1].r);
		em.accumulator += em.recipe.rate * delta;

        if (em.accumulator < 1.0f || em.target_layer == nullptr)
            return;

		const auto offset_count = em.offsets.size();
		while (em.accumulator >= 1.0f)
        {
			auto p = pm.create_particle();
			if (p == nullptr)
				break;

			p->flags = em.recipe.flags;
			p->ry = em.pos.y + em.mirror_offset;
			
			auto offset = Vector2{ 0.f, em.recipe.min_dp.x }.rotate(em.value);
			offset.y = 0.f;
			if (offset_count == 0)
				p->pos = em.pos + offset;
			else
				p->pos = em.pos + em.offsets[random(0, static_cast<int>(offset_count))] + offset;
			
            p->dp.x = offset.x * em.recipe.max_dp.x;
            p->dp.y = -random(em.recipe.min_dp.y, em.recipe.max_dp.y);

			const auto size = random(0.0f, 1.0f);
            p->size = em.recipe.min_size + size * (em.recipe.max_size - em.recipe.min_size);
            p->color = em.recipe.colors[0];
			p->dc = em.recipe.dc * (0.25f + size);
			p->ry = em.pos.y + em.mirror_offset;

            // The smaller the particle, the longer it will live
            p->ttl = em.recipe.min_ttl + (1.f - size) * (em.recipe.max_ttl - em.recipe.min_ttl);

			em.target_layer->add(p);

			em.accumulator -= 1.0f;
        }
    }

	// FOG
	// - some particles escape upwards (amount determined by ratio in color[1].b)
	// - majority of particles drift horizontally (based on dx/dy)
	// - single color that fades over time 
	// - particles emitted within offset[0] - offset[1]
	void fog_update(ParticleManager& pm, ParticleEmitter& em, float delta)
	{
		em.accumulator += em.recipe.rate * delta;

		if (em.accumulator < 1.0f || em.target_layer == nullptr)
			return;

		if (em.offsets.size() != 2)
			return;

		const auto small_threshold = 1.f / em.recipe.colors[1].b;
		const auto min_pos = em.pos + em.offsets[0];
		const auto max_pos = em.pos + em.offsets[1];
		const auto small_min_size = static_cast<int>(em.recipe.min_size * em.recipe.colors[1].r);
		const auto small_max_size = static_cast<int>(em.recipe.max_size * em.recipe.colors[1].g);
		while (em.accumulator >= 1.0f)
		{
			auto p = pm.create_particle();
			if (p == nullptr)
				break;

			em.value += 1.f;

			p->flags = em.recipe.flags;
			p->pos = random(min_pos, max_pos); // spawn within rect

			// most particles are large + slow-moving
			if (em.value < small_threshold)
			{
				p->dp.x = random(em.recipe.min_dp.x, em.recipe.max_dp.x);
				p->dp.y = random(-0.5f, 0.5f); // minimal movement along vertical axis
				const auto size = random(0.0f, 1.0f);
				p->size = std::round(em.recipe.min_size + size * (em.recipe.max_size - em.recipe.min_size));
			}
			else
			{
				p->dp.x = random(em.recipe.min_dp.x, em.recipe.max_dp.x);
				p->dp.y = -random(em.recipe.min_dp.y, em.recipe.max_dp.y);
				p->size = static_cast<float>(random(small_min_size, small_max_size));
				em.value = 0.0f; // reset accumulator
			}

			p->color = em.recipe.colors[0];
			p->dc = em.recipe.dc;
			p->ry = em.pos.y + em.mirror_offset;

			p->ttl = random(em.recipe.min_ttl, em.recipe.max_ttl);
			p->dc.a = -0.5f / p->ttl; // alpha reduction based on ttl

			em.target_layer->add(p);

			em.accumulator -= 1.0f;
		}
	}

	// FOUNTAIN
	// - initial dx, dy used for particle motion
	// - gravity pulls at dy->reduce and ultimately turn around
	void fountain_update(ParticleManager& pm, ParticleEmitter& em, float delta)
    {
		const auto max_change = 0.2f;
		em.value += random(-max_change, max_change);
		em.accumulator += em.recipe.rate * delta;

        if (em.accumulator < 1.0f || em.target_layer == nullptr)
            return;

		const auto offset_count = em.offsets.size();
		const auto max_v = em.recipe.max_dp.y * (1.0f + 0.25f * fast_cos(em.age));
        while (em.accumulator >= 1.0f)
        {
			auto p = pm.create_particle();
			if (p == nullptr)
				break;

			p->flags = em.recipe.flags;
			p->ry = em.pos.y + em.mirror_offset;

			auto offset = Vector2{ 0.f, em.recipe.max_dp.x }.rotate(em.value);
			offset.y = 0.f;
			if (offset_count == 0)
				p->pos = em.pos + offset;
			else
				p->pos = em.pos + em.offsets[random(0, offset_count)] + offset;

			p->dp.x = offset.x;
            p->dp.y = -random(em.recipe.min_dp.y, em.recipe.max_dp.y);
            
            auto size = random(1, em.recipe.colors.size());
            p->size = random(em.recipe.min_size, em.recipe.max_size);
            p->color = em.recipe.colors[size - 1];
			p->dc = em.recipe.dc;
            p->ttl = random(em.recipe.min_ttl, em.recipe.max_ttl);

			em.target_layer->add(p);

			em.accumulator -= 1.0f;
        }
    }

	// EXPLOSION
	// - burst of particles
	// - particle direction determined by random angle
	// - particle position based min_dp.x along direction vector
	// - particle velocity within min_dp.y and max_dp.y along direction
	void explosion_update(ParticleManager& pm, ParticleEmitter& em, float delta)
    {
		if (em.target_layer == nullptr)
            return;

		// value used as repeat interval
		if (em.age > 0.0f)
		{
			if (em.value <= 0.0f)
				return; // no repeat
			else if (em.age < em.value)
				return;
			else
				em.age = 0.0f;
		}

		Vector2 base_offset{ 0, 0 };
		const auto offset_count = static_cast<int>(em.offsets.size());
		if (offset_count > 0)
		{
			// use accumulator to store last offset to avoid repetition
			const auto idx = (static_cast<int>(em.accumulator) + 1) % offset_count;
			base_offset = em.offsets[idx];
			em.accumulator = static_cast<float>(idx);
		}

		const auto scale_delta = em.recipe.max_dp.x - em.recipe.min_dp.x;
		static const Vector2 base_vector{ 0.0f, -1.0f };
		for (auto i = 0; i < static_cast<int>(em.recipe.rate); i++)
		{
			auto p = pm.create_particle();
			if (p == nullptr)
				break;

			p->flags = em.recipe.flags;
			p->ry = em.pos.y + em.mirror_offset;

			const auto angle = random(0.0f, two_pi);
			const auto offset = base_vector.rotate(angle);
			p->pos = em.pos + base_offset;

			if (scale_delta != 0.0f)
			{
				const auto init_scale = random(0.0f, 1.0f);
				p->pos += offset * (em.recipe.min_dp.x + init_scale * scale_delta);
				p->color = lerp(em.recipe.colors[0], em.recipe.colors[1], init_scale);
			}
			else
			{
				p->color = em.recipe.colors[0];
			}
			
			p->dp = offset * random(em.recipe.min_dp.y, em.recipe.max_dp.y);

			const auto n_size = random(0.0f, 1.0f);
			p->size = em.recipe.min_size + n_size * (em.recipe.max_size - em.recipe.min_size);
			p->dc = em.recipe.dc;
			p->dc.a -= n_size;

			// The smaller the particle, the longer it will live
			p->ttl = em.recipe.min_ttl + (1.f - n_size) * (em.recipe.max_ttl - em.recipe.min_ttl);

			em.target_layer->add(p);
		}
    }

	// GROUND_EXPLOSION
	// - burst of particles
	// - particle direction determined by angle within +/- PI/2
	// - particle position based min_dp.x along direction vector
	// - particle velocity within min_dp.y and max_dp.y along direction, 
	//   additionally scaled by angle
	void ground_explosion_update(ParticleManager& pm, ParticleEmitter& em, float delta)
	{
		if (em.target_layer == nullptr)
			return;

		// value used as repeat interval
		if (em.age > 0.0f)
		{
			if (em.value <= 0.0f)
				return; // no repeat
			else if (em.age < em.value)
				return;
			else
				em.age = 0.0f;
		}

		Vector2 base_offset{ 0, 0 };
		const auto offset_count = static_cast<int>(em.offsets.size());
		if (offset_count > 0)
		{
			// use accumulator to store last offset to avoid repetition
			const auto idx = (static_cast<int>(em.accumulator) + 1) % offset_count;
			base_offset = em.offsets[idx];
			em.accumulator = static_cast<float>(idx);
		}

		const auto scale_delta = em.recipe.max_dp.x - em.recipe.min_dp.x;
		static const Vector2 base_vector{ 0.0f, -1.0f };
		for (auto i = 0; i < static_cast<int>(em.recipe.rate); i++)
		{
			auto p = pm.create_particle();
			if (p == nullptr)
				break;

			p->flags = em.recipe.flags;
			p->ry = em.pos.y + em.mirror_offset;

			const auto angle = random(-pi_over_two, pi_over_two);
			const auto offset = base_vector.rotate(angle);
			p->pos = em.pos + base_offset;

			if (scale_delta != 0.0f)
			{
				const auto init_scale = random(0.0f, 1.0f);
				p->pos += offset * (em.recipe.min_dp.x + init_scale * scale_delta);
				p->color = lerp(em.recipe.colors[0], em.recipe.colors[1], init_scale);
			}
			else
			{
				p->color = em.recipe.colors[0];
			}

			// scale dp base on angle - to closer to [0,-1] the faster
			const auto vel = std::abs(offset.y);
			p->dp = offset * random(em.recipe.min_dp.y, em.recipe.max_dp.y) * vel;

			const auto n_size = random(0.0f, 1.0f);
			p->size = em.recipe.min_size + n_size * (em.recipe.max_size - em.recipe.min_size);
			p->dc = em.recipe.dc;
			p->dc.a -= n_size;

			// The smaller the particle, the longer it will live
			p->ttl = em.recipe.min_ttl + (1.f - n_size) * (em.recipe.max_ttl - em.recipe.min_ttl);

			em.target_layer->add(p);
		}
	}

	// BLAST
	// - burst of particles with ramp-up / ramp-down
	// - particle are emitted with dp based on min_dp / max_dp
	// - otherwise behavior similar to LINEAR emitter
	void blast_update(ParticleManager& pm, ParticleEmitter& em, float delta) 
	{
		static constexpr auto delay = 0.25f;
		// Rate is dependent on age:
		// 1. if age < delay, scale between 0 and rate
		// 2. if age > delay, scale between rate and 0 until age > em.value
		float rate;
		if (em.age < delay)
			rate = lerp(0.0f, em.recipe.rate, em.age / delay);
		else
			rate = lerp(em.recipe.rate, 0.0f, (em.age - delay) * (em.age - delay) / em.value);

		em.accumulator += rate * delta;
		if (em.accumulator < 1.0f || em.target_layer == nullptr)
			return;

		const auto offset_count = em.offsets.size();
		while (em.accumulator >= 1.0f)
		{
			auto p = pm.create_particle();
			if (p == nullptr)
				return;
			p->flags = em.recipe.flags;
			p->ry = em.pos.y + em.mirror_offset;

			if (offset_count == 0)
				p->pos = em.pos;
			else
				p->pos = em.pos + em.offsets[random(0, offset_count)];

			p->dp = random(em.recipe.min_dp, em.recipe.max_dp);
			p->size = random(em.recipe.min_size, em.recipe.max_size);
			p->color = em.recipe.colors[random(0, em.recipe.colors.size())];
			p->dc = em.recipe.dc;
			p->ttl = random(em.recipe.min_ttl, em.recipe.max_ttl);

			em.target_layer->add(p);

			em.accumulator -= 1.0f;
		}
	}

	// SPIRAL
	// - spiralling emitters
	// - angle changes with time, particles are emitted with initial direction based on angle
	// - min_dp.x is radious around center at which to emit
	// - min_dy.y is the value increment
	void spiral_update(ParticleManager& pm, ParticleEmitter& em, float delta)
	{
		em.value += em.recipe.min_dp.y * delta; 
		em.accumulator += em.recipe.rate * delta;

		if (em.accumulator < 1.0f || em.target_layer == nullptr)
			return;

		const auto offset = Vector2{ 0.f, 1.f }.rotate(em.value);

		const auto offset_count = em.offsets.size();
		while (em.accumulator >= 1.0f)
		{
			auto p = pm.create_particle();
			if (p == nullptr)
				break;

			p->flags = em.recipe.flags;
			p->ry = em.pos.y + em.mirror_offset;
			p->dp.x = em.recipe.max_dp.x * offset.x;
			p->dp.y = em.recipe.max_dp.y * offset.y;

			p->pos = em.pos + offset * em.recipe.min_dp.x;
			if (offset_count > 0)
				p->pos += em.offsets[random(0, offset_count)];

			const auto n_size = random(0.0f, 1.0f);
			p->size = em.recipe.min_size + n_size * (em.recipe.max_size - em.recipe.min_size);

			p->color = em.recipe.colors[0];
			p->dc = em.recipe.dc;
			p->dc.a -= n_size;

			// The smaller the particle, the longer it will live
			p->ttl = em.recipe.min_ttl + (1.f - n_size) * (em.recipe.max_ttl - em.recipe.min_ttl);

			em.target_layer->add(p);

			em.accumulator -= 1.0f;
		}
	}

	void init_emitter(ParticleEmitter& emitter, const ParticleEmitter::Recipe& recipe)
	{
		emitter.recipe = recipe;
		switch (recipe.type)
		{
		case ParticleEmitter::Recipe::Linear:
			emitter.update = std::bind(linear_update, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			break;
		case ParticleEmitter::Recipe::Uniform:
			emitter.update = std::bind(uniform_update, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			break;
		case ParticleEmitter::Recipe::Flame:
			emitter.update = std::bind(flame_update, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			break;
		case ParticleEmitter::Recipe::Smoke:
			emitter.update = std::bind(smoke_update, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			break;
		case ParticleEmitter::Recipe::Fog:
			emitter.update = std::bind(fog_update, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			break;
		case ParticleEmitter::Recipe::Fountain:
			emitter.update = std::bind(fountain_update, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			break;
		case ParticleEmitter::Recipe::Explosion:
			emitter.update = std::bind(explosion_update, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			break; 
		case ParticleEmitter::Recipe::GroundExplosion:
			emitter.update = std::bind(ground_explosion_update, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			break;
		case ParticleEmitter::Recipe::Spiral:
			emitter.update = std::bind(spiral_update, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			break;
		case ParticleEmitter::Recipe::Radial:
			emitter.update = std::bind(radial_update, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			break;
		case ParticleEmitter::Recipe::Layered:
			emitter.update = std::bind(layered_update, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			break;
		case ParticleEmitter::Recipe::Blast:
			emitter.update = std::bind(blast_update, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
			break;
		default:
			emitter.update = nullptr;
			break;
		}		
	}
}