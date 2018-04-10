#include "stdafx.h"
#include "boid.h"

namespace dukat
{
	const float neighbor_distance = 50.0f;
	const float separation_distance = 25.0f;

	Boid::Boid(Particle * p, bool predator) : p(p), predator(predator), max_speed(120.0f), max_force(0.8f)
	{
		auto angle = randf(0, two_pi);
		p->dp.rotate(angle);
	}

	void Boid::update(const std::vector<Boid>& boids)
	{
		// Flocking
		auto sep = seperate(boids);
		auto ali = align(boids);
		auto coh = cohesion(boids);
		// Weigh & combine
		auto acceleration = (sep * 1.5f) + (ali * 1.0f) + (coh * 1.0f);
		p->dp += acceleration;
		p->dp.limit(max_speed);

		if (predator && randf(0.0, 1.0) < 0.001f)
			p->dp = -p->dp;
	}

	Vector2 Boid::seperate(const std::vector<Boid>& boids) const
	{
		Vector2 steer{};

		if (predator)
			return steer;

		auto count = 0;
		for (const auto& boid : boids) 
		{
			// compute vector pointing away from other
			auto diff = (p->pos - boid.p->pos);
			auto dist = diff.mag();
			if (dist > 0.0f && dist < separation_distance)
			{
				// normalize & weight by distance
				if (!boid.predator) 
				{
					diff /= dist * dist;
				}
				steer += diff;
				count++;
			}
		}

		if (count > 0)
		{
			steer /= static_cast<float>(count);
			if (steer.mag2() > 0.0f)
			{
				steer.set_mag(max_speed);
				steer -= p->dp;
				steer.limit(max_force);
			}
		}

		return steer;
	}

	Vector2 Boid::align(const std::vector<Boid>& boids) const
	{
		Vector2 sum{};
		auto count = 0;
		for (auto boid : boids)
		{
			if (boid.predator)
				continue;

			auto diff = (p->pos - boid.p->pos).mag();
			if (diff > 0.0f && diff < neighbor_distance)
			{
				sum += boid.p->dp;
				count++;
			}
		}

		if (count > 0)
		{
			sum /= static_cast<float>(count);
			sum.set_mag(max_speed);
			auto steer = sum - p->dp;
			steer.limit(max_force);
			return steer;
		}
		else
		{
			return Vector2{};
		}
	}

	Vector2 Boid::cohesion(const std::vector<Boid>& boids) const
	{
		Vector2 sum{};
		auto count = 0;

		for (auto boid : boids)
		{
			if (boid.predator)
				continue;

			auto diff = (p->pos - boid.p->pos).mag();
			if (diff > 0.0f && diff < neighbor_distance)
			{
				sum += boid.p->pos;
				count++;
			}
		}

		if (count > 0)
		{
			auto steer = sum / static_cast<float>(count) - p->pos;
			steer.set_mag(max_speed);
			steer -= p->dp;
			steer.limit(max_force);
			return steer;
		}
		else
		{
			return Vector2{};
		}
	}
}