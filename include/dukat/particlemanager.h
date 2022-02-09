#pragma once

#include <list>
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
		// Object pools
		std::list<std::unique_ptr<Particle>> particles;
		std::list<std::unique_ptr<ParticleEmitter>> emitters;
		// Gravitational constant applied to particles' vertical motion.
		float gravity;
		// Dampening factor.
		float dampening;

		void update_particles(float delta);
		void update_emitters(float delta);

	public:
		ParticleManager(GameBase* game) : Manager(game), gravity(25.0f), dampening(0.99f) { }
		~ParticleManager(void) { }

		void set_gravity(float gravity) { this->gravity = gravity; }
		void set_dampening(float dampening) { this->dampening = dampening; }

		// Updates all particles position in space.
		void update(float delta) { update_emitters(delta); update_particles(delta); }
		// Creates a new particle.
		Particle* create_particle(void);
		// Creates a new particle emitter from a recipe. May return null if pool is at capacity.
		ParticleEmitter* create_emitter(const ParticleEmitter::Recipe& recipe);
		// Frees up a particle emitter.
		void remove_emitter(ParticleEmitter* emitter);
		// Removes all particles and emitters
		void clear(void);
	};
}
