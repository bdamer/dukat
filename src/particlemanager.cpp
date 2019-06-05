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
		for (auto it = emitters.begin(); it != emitters.end(); )
		{
			auto& e = (*it);
			if (e->active)
				e->update(this, delta);
			e->age += delta;
			if (e->ttl > 0.0f && e->age >= e->ttl)
				it = emitters.erase(it);
			else
				++it;
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
		std::unique_ptr<ParticleEmitter> emitter;
		switch (recipe.type)
		{
			case ParticleEmitter::Recipe::Linear:
				emitter = std::make_unique<LinearEmitter>(recipe);
				break;
			case ParticleEmitter::Recipe::Flame:
				emitter = std::make_unique<FlameEmitter>(recipe);
				break;
			case ParticleEmitter::Recipe::Smoke:
				emitter = std::make_unique<SmokeEmitter>(recipe);
				break;
			case ParticleEmitter::Recipe::Fountain:
				emitter = std::make_unique<FountainEmitter>(recipe);
				break;
			case ParticleEmitter::Recipe::Explosion:
				emitter = std::make_unique<ExplosionEmitter>(recipe);
				break;
			default:
				emitter = nullptr;
				break;
		}

		if (emitter == nullptr)
			return nullptr;

		auto res = emitter.get();
		emitters.push_back(std::move(emitter)); 
		return res;
	}

	void ParticleManager::remove_emitter(ParticleEmitter* emitter) 
	{ 
		auto it = std::find_if(emitters.begin(), emitters.end(), [emitter](const std::unique_ptr<ParticleEmitter>& e) {
			return e.get() == emitter;
		});
		if (it != emitters.end())
			emitters.erase(it);
	}
}
