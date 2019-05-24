#pragma once

#include <deque>
#include <memory>

#include "particle.h"
#include "particleemitter.h"
#include "manager.h"

namespace dukat
{
	// Manager in charge of all particles on screen. 
	class ParticleManager : public Manager
	{
	private:
		// Global limit to number of particles. Once that limit is hit, the system will
		// recycle old particles.
		static const int max_particles = 1000;
		// Gravitational constant applied to particles' vertical motion.
		float gravity;
		// Dampening factor.
		float dampening;
		// Particle list
		std::deque<std::unique_ptr<Particle>> particles;
		// List of registered emitters
		std::vector<std::unique_ptr<ParticleEmitter>> emitters;

		// Internal method to artifically age particles at the end of the queue
		void free_particle(void);

	public:
		ParticleManager(GameBase* game) : Manager(game), gravity(25.0f), dampening(0.99f) { }
		~ParticleManager(void) { }

		// Updates all particles position in space.
		void update(float delta);
		// Creates a new particle.
		Particle* create_particle(void);
		// Adds an existing partilce to be managed us.
		void add_particle(std::unique_ptr<Particle> particle);
		// Removes all particles.
		void clear(void);

		void set_gravity(float gravity) { this->gravity = gravity; }
		void set_dampening(float dampening) { this->dampening = dampening; }

		ParticleEmitter* add_emitter(std::unique_ptr<ParticleEmitter> emitter) { emitters.push_back(std::move(emitter)); }
		void remove_emitter(ParticleEmitter* emitter);	
	};
}
