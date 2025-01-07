#include "stdafx.h"
#include <dukat/particlemanager.h>
#include <dukat/particleemitter.h>
#include <dukat/bit.h>
#include <dukat/log.h>
#include <dukat/mathutil.h>
#include <dukat/perfcounter.h>

namespace dukat
{
	ParticleManager::ParticleManager(GameBase* game) : Manager(game), gravity(60.0f), dampening(0.99f) 
	{ 
	}

	void ParticleManager::clear(void)
	{
		for (auto& p : particles)
			p->ttl = -1.0f; // allow particle to be deallocated during next update cycle
		emitters.clear();
	}

	void ParticleManager::update_particles(float delta)
	{
		for (auto it = particles.begin(); it != particles.end(); )
		{
			auto& p = *(*it);
			if (!check_flag(p.flags, Particle::Alive))
			{
				it = particles.erase(it);
				continue;
			}

			if (p.ttl <= 0.0f)
			{
				// mark particle as dead for removal during next frame
				p.flags &= ~Particle::Alive;
			}
			else
			{
				p.ttl -= delta;
				if (check_flag(p.flags, Particle::Linear))
				{
					p.pos += p.dp * delta;
				}
				else if (check_flag(p.flags, Particle::Spiraling))
				{
					p.angle += p.dp.x * delta;
					p.pos.x = p.ref.x + p.radius * fast_cos(p.angle);
					p.pos.y += p.dp.y * delta;

					// reduce angular speed over time
					const auto reduction = delta * 0.125f;
					p.dp.x = p.dp.x > 0.f ? std::max(0.1f, p.dp.x - reduction) : std::min(-0.1f, p.dp.x + reduction);
					p.radius += 10.f * delta;
				}

				// For gravitational particles, only apply effect as long as we're above
				// reflection line
				if (check_flag(p.flags, Particle::Gravitational))
				{
					if (p.pos.y < p.ref.y)
						p.dp.y += gravity * delta;
					else
					{
						p.pos.y = p.ref.y;
						// zero out dp once we reach reflection line
						p.dp.x = p.dp.y = 0.f;
					}
				}
				else if (check_flag(p.flags, Particle::AntiGravitational))
				{
					p.dp.y -= gravity * delta;
				}

				if (check_flag(p.flags, Particle::Dampened))
					p.dp *= dampening;

				p.color += p.dc * delta;
				p.size += p.dsize * delta;
			}

			++it;
		}
		perfc.inc(PerformanceCounter::PARTICLES_TOTAL, static_cast<int>(particles.size()));
	}

	void ParticleManager::update_emitters(float delta)
	{
		for (auto it = emitters.begin(); it != emitters.end(); )
		{
			auto& e = *(*it);
			if (e.active)
			{
				e.update(*this, e, delta);
				e.age += delta;
			}
			if (e.ttl > 0.0f && e.age >= e.ttl)
				it = emitters.erase(it);
			else
				++it;
		}
		perfc.inc(PerformanceCounter::EMITTERS, emitters.size());
	}

	Particle* ParticleManager::create_particle(void)
	{
		auto p = std::make_unique<Particle>();
		auto res = p.get();
		particles.push_back(std::move(p));
		return res;
	}

	ParticleEmitter* ParticleManager::create_emitter(const ParticleRecipe& recipe)
	{
		auto emitter = std::make_unique<ParticleEmitter>();
		init_emitter(*emitter, recipe);
		auto res = emitter.get();
		emitters.push_back(std::move(emitter));
		return res;
	}

	void ParticleManager::remove_emitter(ParticleEmitter* emitter) 
	{
		auto it = std::find_if(emitters.begin(), emitters.end(),
			[&](const std::unique_ptr<ParticleEmitter>& e) { return e.get() == emitter; });
		if (it != emitters.end())
			emitters.erase(it);
	}
}
