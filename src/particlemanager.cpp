#include "stdafx.h"
#include "particlemanager.h"

namespace dukat
{
	Particle* ParticleManager::create_particle(void)
	{
		auto particle = std::make_unique<Particle>();
		auto res = particle.get();
		if (particles.size() == max_particles)
		{
			particles.pop_back();
		}
		particles.push_front(std::move(particle));
		return res;
	}

	void ParticleManager::add_particle(std::unique_ptr<Particle> particle)
	{
		if (particles.size() == max_particles)
		{
			particles.pop_back();
		}
		particles.push_front(std::move(particle));
	}

	void ParticleManager::update(float delta)
	{
		for (auto it = particles.begin(); it != particles.end(); )
		{
			if ((*it)->ttl <= 0.0f)
			{
				it = particles.erase(it);
			}
			else
			{
				(*it)->ttl -= delta;
				(*it)->pos += (*it)->dp * delta;
				(*it)->color += (*it)->dc * delta;
				(*it)->size += (*it)->dsize * delta;
				++it;
			}
		}
	}
}
