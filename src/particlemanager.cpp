#include "stdafx.h"
#include <dukat/particlemanager.h>
#include <dukat/particleemitter.h>
#include <dukat/bit.h>
#include <dukat/log.h>
#include <dukat/perfcounter.h>

namespace dukat
{
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
					p.pos += p.dp * delta;
				// For gravitational particles, only apply effect as long as we're above
				// reflection line
				if (check_flag(p.flags, Particle::Gravitational))
				{
					if (p.pos.y < p.ry)
						p.dp.y += gravity * delta;
					else
						p.pos.y = p.ry;
				}
				if (check_flag(p.flags, Particle::Dampened))
					p.dp *= dampening;
				p.color += p.dc * delta;
				p.size += p.dsize * delta;

			}

			++it;
		}
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

	ParticleEmitter* ParticleManager::create_emitter(const ParticleEmitter::Recipe& recipe)
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
