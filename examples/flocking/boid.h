#pragma once

#include <dukat/dukat.h>

namespace dukat
{
	struct Boid
	{
		float max_speed; // max speed
		float max_force; // max steering force
		bool predator;
		Particle* p;

		Boid(Particle* p, bool predator = false);
		~Boid(void) { }

		void update(const std::vector<Boid>& boids);
		// Steer away from nearby boids.
		virtual Vector2 seperate(const std::vector<Boid>& boids) const;
		virtual Vector2 align(const std::vector<Boid>& boids) const;
		virtual Vector2 cohesion(const std::vector<Boid>& boids) const;
	};
}