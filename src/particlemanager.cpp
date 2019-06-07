#include "stdafx.h"
#include <dukat/particlemanager.h>
#include <dukat/particleemitter.h>
#include <dukat/bit.h>
#include <dukat/log.h>

namespace dukat
{
	void ParticleManager::clear(void)
	{
		for (auto& p : particles.data)
			p.ttl = -1.0f;
		emitters.clear();
	}

	void ParticleManager::update(float delta)
	{
		emitters.free_index = 0;
		for (auto& e : emitters.data)
		{
			if (!e.alive)
				continue;
			if (e.active)
			{
				e.update(*this, e, delta);
				e.age += delta;
			}
			if (e.ttl > 0.0f && e.age >= e.ttl)
				emitters.release(e);
		}

		// Reset free index
		particles.free_index = 0;
		for (auto& p : particles.data)
		{
			if (!check_flag(p.flags, Particle::Alive))
				continue;

			if (p.ttl <= 0.0f)
			{
				particles.release(p);
			}
			else
			{
				p.ttl -= delta;
				if (check_flag(p.flags, Particle::Linear))
					p.pos += p.dp * delta;
				if (check_flag(p.flags, Particle::Gravitational))
					p.dp.y += gravity * delta;
				if (check_flag(p.flags, Particle::Dampened))
					p.dp *= dampening;
				p.color += p.dc * delta;
				p.size += p.dsize * delta;
			}
		}
	}

	ParticleEmitter* ParticleManager::create_emitter(const ParticleEmitter::Recipe& recipe)
	{
		auto emitter = emitters.acquire();
		if (emitter == nullptr)
			return nullptr;
		init_emitter(*emitter, recipe);
		return emitter;
	}

	void ParticleManager::remove_emitter(ParticleEmitter* emitter) 
	{
		emitters.release(emitter);
	}
}
