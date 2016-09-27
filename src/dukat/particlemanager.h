#pragma once

#include <deque>
#include <memory>
#include "particle.h"

namespace dukat
{
	// Manager in charge of all particles on screen. 
	class ParticleManager
	{
	private:
		// Global limit to number of particles. Once that limit is hit, the system will
		// recycle old particles.
		static const int max_particles = 1000;
		std::deque<std::unique_ptr<Particle>> particles;

	public:
		ParticleManager(void) { }
		~ParticleManager(void) { }

		// Updates all particles position in space.
		void update(float delta);
		// Creates a new particle.
		Particle* create_particle(void);
		// Adds an existing partilce to be managed us.
		void add_particle(std::unique_ptr<Particle> particle);
	};
}
