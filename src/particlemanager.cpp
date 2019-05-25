#include "stdafx.h"
#include <dukat/particlemanager.h>
#include <dukat/particleemitter.h>
#include <dukat/bit.h>
#include <dukat/log.h>

namespace dukat
{
	Particle* ParticleManager::create_particle(void)
	{
		auto particle = std::make_unique<Particle>();
		auto res = particle.get();
		if (particles.size() == max_particles)
			free_particle();
		particles.push_front(std::move(particle));
		return res;
	}

	void ParticleManager::add_particle(std::unique_ptr<Particle> particle)
	{
		if (particles.size() == max_particles)
			free_particle();
		particles.push_front(std::move(particle));
	}

	void ParticleManager::free_particle(void)
	{
		for (auto it = particles.rbegin(); it != particles.rend(); ++it)
		{
			if ((*it)->ttl > 0.0f)
			{
				(*it)->ttl = -1.0f;
				return;
			}
		}
	}

	void ParticleManager::clear(void)
	{
		for (auto& p : particles)
			p->ttl = -1.0f;
		emitters.clear();
	}

	void ParticleManager::update(float delta)
	{
		for (auto& e : emitters)
		{
			if (e->active)
				e->update(this, delta);
		}

		for (auto it = particles.begin(); it != particles.end(); )
		{
			if ((*it)->ttl <= 0.0f)
			{
				it = particles.erase(it);
			}
			else
			{
				auto& p = *it;
				p->ttl -= delta;
				if (check_flag(p->flags, Particle::Linear))
					p->pos += p->dp * delta;
				if (check_flag(p->flags, Particle::Gravitational))
					p->dp.y += gravity * delta;
				if (check_flag(p->flags, Particle::Dampened))
					p->dp *= dampening;
				(*it)->color += (*it)->dc * delta;
				(*it)->size += (*it)->dsize * delta;
				++it;
			}
		}
	}

	ParticleEmitter* ParticleManager::create_emitter(const ParticleEmitter::Recipe& recipe)
	{
		std::unique_ptr<ParticleEmitter> emitter;
		switch (recipe.type)
		{
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
