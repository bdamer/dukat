#pragma once

#include <deque>
#include <memory>

#include "objectpool.h"
#include "particle.h"
#include "particleemitter.h"
#include "manager.h"

namespace dukat
{
	// Manager in charge of all particles on screen. 
	class ParticleManager : public Manager
	{
	private:
		struct ParticleAllocator
		{
			void init(Particle& p) 
			{ 
				p.flags = Particle::Alive | Particle::Linear; 
				p.dp.x = p.dp.y = p.dc.r = p.dc.g = p.dc.b = p.dc.a = p.dsize = 0.0f; 
			}
			bool is_alive(Particle& p) { return (p.flags & Particle::Alive) == Particle::Alive; }
			void free(Particle& p) { p.flags = 0; }
		};

		struct ParticleEmitterAllocator
		{
			void init(ParticleEmitter& em) 
			{ 
				em.active = em.alive = true; 
				em.accumulator = em.age = em.mirror_offset = em.ttl = em.value = 0.0f;
				em.offsets.clear();
				em.target_layer = nullptr;
				em.update = nullptr;
			}
			bool is_alive(ParticleEmitter& em) { return em.alive; }
			void free(ParticleEmitter& em) { em.alive = false; }
		};

		// Global limit to number of particles.
		static constexpr auto max_particles = 4096;
		static constexpr auto max_emitters = 256;
		// Particle pool
		ObjectPool<Particle, max_particles, ParticleAllocator> particles;
		// Emitter pool
		ObjectPool<ParticleEmitter, max_emitters, ParticleEmitterAllocator> emitters;

		// Gravitational constant applied to particles' vertical motion.
		float gravity;
		// Dampening factor.
		float dampening;

	public:
		ParticleManager(GameBase* game) : Manager(game), gravity(25.0f), dampening(0.99f) { }
		~ParticleManager(void) { }

		void set_gravity(float gravity) { this->gravity = gravity; }
		void set_dampening(float dampening) { this->dampening = dampening; }

		// Updates all particles position in space.
		void update(float delta);
		// Creates a new particle. May return null if pool is at capacity.
		Particle* create_particle(void) { return particles.acquire(); }
		// Creates a new particle emitter from a recipe. May return null if pool is at capacity.
		ParticleEmitter* create_emitter(const ParticleEmitter::Recipe& recipe);
		// Frees up a particle emitter.
		void remove_emitter(ParticleEmitter* emitter);
		// Removes all particles and emitters
		void clear(void);
	};
}
