#include "stdafx.h"
#include <dukat/particlerecipe.h>

namespace dukat
{
	namespace recipes
	{
		const ParticleRecipe FlameRecipe = ParticleRecipeBuilder()
			.type(ParticleRecipe::Flame)
			.flags(Particle::Alive | Particle::Linear)
			.rate(400.0f)
			.size(1.0f, 6.0f)
			.ttl(1.0f, 5.0f)
			.dp(Vector2{ 2, 25 }, Vector2{ 2, 40 })
			.colors({
				Color{ 1.0f, 1.0f, 0.0f, 1.0f },	// Center color
				Color{ 0.0f, 0.25f, 0.0f, 0.0f },	// Color reduction as we move away from center
				Color{ 0.0f, 0.0f, 0.0f, 0.0f },	// Not used
				Color{ 0.0f, 0.0f, 0.0f, 0.0f },	// Not used
				})
				.dc(Color{ 0.0f, -0.5f, 0.0f, -0.05f }) // Color reduction over time
			.build();

		const ParticleRecipe SmokeRecipe = ParticleRecipeBuilder()
			.type(ParticleRecipe::Smoke)
			.flags(Particle::Alive | Particle::Linear)
			.rate(100.0f)
			.size(4.0f, 8.0f)
			.ttl(2.0f, 6.0f)
			// min_dp.x used to determine initial range
			// max_dp.x used to scale particle motion 
			.dp(Vector2{ 4, 15 }, Vector2{ 1, 25 })
			.colors({
				Color{ 1.0f, 1.0f, 1.0f, 1.0f },	// Smoke color
				Color{ 0.15f, 0.0f, 0.0f, 0.0f },	// R-value: rate of change to angle, other values not used
				Color{ 0.0f, 0.0f, 0.0f, 0.0f },	// Not used
				Color{ 0.0f, 0.0f, 0.0f, 0.0f }		// Not used
				}
			)
			.dc(Color{ 0.f, 0.f, 0.f, -0.5f })		// Color reduction over time
			.build();

		const ParticleRecipe FogRecipe = ParticleRecipeBuilder()
			.type(ParticleRecipe::Fog)
			.flags(Particle::Alive | Particle::Linear)
			.rate(40.0f)
			.size(8.0f, 16.0f)  // min/max size of ground particles
			.ttl(5.0f, 10.0f)
			// _dp.x used for particle motion
			// _dp.y used for particle motion that escape fog bank
			.dp(Vector2{ -2, 2 }, Vector2{ 2, 4 })
			.colors({
				Color{ 1.0f, 1.0f, 1.0f, 0.85f },	// Fog color
				Color{ 0.75f, 0.75f, 0.1f, 0.0f },	// R/G - min/max scalar coefficient of small particles 
													// B - Emit ratio for small particles
				Color{ 0.0f, 0.0f, 0.0f, 0.0f },	// Not used
				Color{ 0.0f, 0.0f, 0.0f, 0.0f },	// Not used
				})
				.dc(Color{ 0.0f, 0.0f, 0.0f, 0.0f })	// Color reduction over time
			.build();

		const ParticleRecipe FountainRecipe = ParticleRecipeBuilder()
			.type(ParticleRecipe::Fountain)
			.flags(Particle::Alive | Particle::Linear | Particle::Gravitational)
			.rate(200.0f)
			.size(1.0f, 4.0f)
			.ttl(4.0f, 6.0f)
			// x used for initial range
			// y used for initial motion in -y direction
			.dp(Vector2{ 0, 80 }, Vector2{ 4, 100 })
			.colors({
			// Each color picked randomly
			Color{ 0.47f, 0.945f, 1.0f, 0.8f },
			Color{ 0.0f, 0.8f, 0.976f, 0.866f },
			Color{ 0.0f, 0.596f, 0.862f, 0.933f },
			Color{ 0.0f, 0.411f, 0.666f, 1.0f }
				})
			.dc(Color{ 0.0f, 0.0f, 0.0f, -0.1f }) // Color reduction over time
			.build();

		const ParticleRecipe ExplosionRecipe = ParticleRecipeBuilder()
			.type(ParticleRecipe::Explosion)
			.flags(Particle::Alive | Particle::Linear | Particle::Dampened)
			.rate(100.0f)
			.size(1.0f, 6.0f)
			.ttl(1.0f, 5.0f)
			// x used to adjust initial position in particle direction 
			// y used to determine particle velocity
			.dp(Vector2{ 0, 25 }, Vector2{ 10, 35 })
			.colors({
				Color{ 1.0f, 0.78f, 0.14f, 1.0f },	// Inner color (at min_dp.x)
				Color{ 1.0f, 0.31f, 0.0f, 1.0f },	// Outer color (at max_dp.x)
				Color{ 0.0f, 0.0f, 0.0f, 0.0f },	// Not used
				Color{ 0.0f, 0.0f, 0.0f, 0.0f }		// Not used
				})
			.dc(Color{ 0.0f, -0.25f, -0.05f, -0.05f })
			.build();

		const ParticleRecipe GroundExplosionRecipe = ParticleRecipeBuilder()
			.type(ParticleRecipe::GroundExplosion)
			.flags(Particle::Alive | Particle::Linear | Particle::Gravitational)
			.rate(100.0f)
			.size(1.0f, 6.0f)
			.ttl(1.0f, 5.0f)
			// x used to adjust initial position in particle direction 
			// y used to determine particle velocity
			.dp(Vector2{ 0, 25 }, Vector2{ 10, 35 })
			.colors({
				Color{ 1.0f, 0.78f, 0.14f, 1.0f },	// Inner color (at min_dp.x)
				Color{ 1.0f, 0.31f, 0.0f, 1.0f },	// Outer color (at max_dp.x)
				Color{ 0.0f, 0.0f, 0.0f, 0.0f },	// Not used
				Color{ 0.0f, 0.0f, 0.0f, 0.0f }		// Not used
				})
			.dc(Color{ 0.0f, -0.25f, -0.05f, -0.05f })  // Color reduction over time
			.build();

		const ParticleRecipe SpiralRecipe = ParticleRecipeBuilder()
			.type(ParticleRecipe::Spiral)
			.flags(Particle::Alive | Particle::Linear)
			.rate(40.0f)
			.size(1.f, 4.f)
			.ttl(1.f, 5.f)
			.dp(Vector2{ 4, 3 },	// x used to scale emit range, y used to define max angular change per second
				Vector2{ 64, 64 }	// Used to scale dp
			)
			.colors({
				Color{ 1.0f, 1.0f, 0.0f, 1.0f },	// Initial color
				Color{ 0.0f, 0.0f, 0.0f, 0.0f },	// Not used
				Color{ 0.0f, 0.0f, 0.0f, 0.0f },	// Not used
				Color{ 0.0f, 0.0f, 0.0f, 0.0f }		// Not used
				})
			.dc(Color{ 0.0f, -1.0f, 0.0f, -0.1f }) // Color reduction over time
			.build();

		const ParticleRecipe SnowRecipe = ParticleRecipeBuilder()
			.type(ParticleRecipe::Layered)
			.flags(Particle::Alive | Particle::Linear)
			.rate(40.0f)
			.size(1.0f, 3.0f)	// size within 1,3
			.ttl(10.0f, 10.0f)
			.dp(Vector2{ -2, 24 },	// particle motion for background (color[2] and color[3])
				Vector2{ 4, 36 })	// particle motion for foreground (color[0] and color[1])
			.colors({
				color_rgba(0xffffffff),
				color_rgba(0xffffffc0),
				color_rgba(0xc7cfddc0),
				color_rgba(0x94fdffc0)
				})
			.dc(Color{ 0.f, 0.f, 0.f, -0.005f })
			.build();

		const ParticleRecipe BlastRecipe = ParticleRecipeBuilder()
			.type(ParticleRecipe::Blast)
			.flags(Particle::Alive | Particle::Linear | Particle::AntiGravitational)
			.rate(100.0f)
			.size(1.0f, 4.0f)
			.ttl(1.0f, 3.0f)
			.dp(Vector2{ 32, -7 }, Vector2{ 64, 7 }) // direction
			.colors({
				color_rgb(0xffc825),
				color_rgb(0xffa214),
				color_rgb(0xed7614),
				color_rgb(0xff5000)
				})
			.dc(Color{ 0.1f, -0.1f, -0.1f, -0.25f })
			.build();

		const ParticleRecipe HelixRecipe = ParticleRecipeBuilder()
			.type(ParticleRecipe::Helix)
			.flags(Particle::Alive | Particle::Linear)
			.rate(40.0f)
			.size(2.0f, 4.0f)
			.ttl(1.0f, 5.0f)
			.dp(Vector2{ 30, 8 },	// x used to scale emit range, y used to define max angular change per second
				Vector2{ 0, -40 }	// Used to scale dp
			)
			.colors({
				Color{ 0.78f, 0.81f, 0.87f, 1.0f },	// Initial color
				Color{ 0.0f, 0.0f, 0.0f, 0.0f },	// Not used
				Color{ 0.0f, 0.0f, 0.0f, 0.0f },	// Not used
				Color{ 0.0f, 0.0f, 0.0f, 0.0f },	// Not used
				})
				.dc(Color{ 0.0f, -0.5f, 0.0f, -0.075f }) // Color reduction over time
			.build();

		const ParticleRecipe VortexRecipe = ParticleRecipeBuilder()
			.type(ParticleRecipe::Vortex)
			.flags(Particle::Alive | Particle::Spiraling)
			.rate(32.0f)
			.radius(24.0f)
			.size(1.0f, 1.0f, 0.25f)
			.ttl(7.5f, 8.0f)
			.dp(Vector2{ 8.0f, -12.f }, Vector2{ 12.0f, -24.f })
			.colors({ 
				color_rgb(0x94fdff), 
				color_rgb(0x0cf1ff), 
				color_rgb(0x00cdf9), 
				color_rgb(0x0098dc) 
			})
			.dc(Color{ 0.f, 0.f, 0.f, -0.04f })
			.build();
	}
}