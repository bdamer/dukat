#pragma once

#include <array>
#include "color.h"
#include "particle.h"
#include "vector2.h"

namespace dukat
{
	struct ParticleRecipe
	{
		// Recipe for a particle emitter.
		enum Type
		{
			None,
			Linear,
			Uniform,
			Flame,
			Smoke,
			Fountain,
			Explosion,
			Spiral,
			Radial,
			Layered,
			Blast,
			GroundExplosion,
			Fog,
			Helix,
			Vortex,
		};

		Type type;
		// Flags of particles to create.
		uint8_t flags;
		// Rate of particle emission (particles / second)
		float rate;
		// Radius for spiral motion
		float radius;
		// particle size
		float min_size;
		float max_size;
		float dsize;
		// particle ttl
		float min_ttl;
		float max_ttl;
		// particle speed
		Vector2 min_dp;
		Vector2 max_dp;
		// particle colors
		std::array<Color, 4> colors;
		// color reduciton over time
		Color dc;

		ParticleRecipe(void) : type(None), flags(Particle::Alive | Particle::Linear), rate(0.0f),
			min_size(0.0f), max_size(0.0f), dsize(0.0f), min_ttl(0.0f), max_ttl(0.0f), colors() {}
		~ParticleRecipe(void) {}
	};

	class ParticleRecipeBuilder
	{
	private:
		ParticleRecipe recipe;

	public:
		ParticleRecipeBuilder(void) { }
		ParticleRecipeBuilder(const ParticleRecipe& recipe) : recipe(recipe) { }
		~ParticleRecipeBuilder(void) { }

		ParticleRecipeBuilder& type(ParticleRecipe::Type type) { recipe.type = type; return *this; }
		ParticleRecipeBuilder& flags(uint8_t flags) { recipe.flags = flags; return *this; }
		ParticleRecipeBuilder& rate(float rate) { recipe.rate = rate; return *this; }
		ParticleRecipeBuilder& radius(float radius) { recipe.radius = radius; return *this; }
		ParticleRecipeBuilder& size(float min_size, float max_size, float dsize = 0.0f) { recipe.min_size = min_size; recipe.max_size = max_size; recipe.dsize = dsize; return *this; }
		ParticleRecipeBuilder& dsize(float dsize) { recipe.dsize = dsize; return *this; }
		ParticleRecipeBuilder& dp(const Vector2& min_dp, const Vector2& max_dp) { recipe.min_dp = min_dp; recipe.max_dp = max_dp; return *this; }
		ParticleRecipeBuilder& min_dp(const Vector2& min_dp) { recipe.min_dp = min_dp; return *this; }
		ParticleRecipeBuilder& max_dp(const Vector2& max_dp) { recipe.max_dp = max_dp; return *this; }
		ParticleRecipeBuilder& ttl(float min_ttl, float max_ttl) { recipe.min_ttl = min_ttl; recipe.max_ttl = max_ttl; return *this; }
		ParticleRecipeBuilder& colors(const std::array<Color, 4>& colors) { recipe.colors = colors; return *this; }
		ParticleRecipeBuilder& dc(const Color& dc) { recipe.dc = dc; return *this; }

		ParticleRecipe build(void) { return recipe; }
	};

	// Default recipes
	namespace recipes
	{
		// Flame particles emitter
		// default params: rate,size,ttl,dc
		// emit distance: min_dp.x
		// horizontal motion: max_dp.x
		// vertical motion: [min_dp.y, max_dp.y]
		// colors[0]: center color
		// colors[1]: outer color
		extern const ParticleRecipe FlameRecipe;

		// Smoke particles emitter
		// default params: rate,size,ttl,dc
		// emit distance: min_dp.x
		// horizontal motion: max_dp.x
		// vertical motion: [min_dp.y, max_dp.y]
		// colors[0]: Smoke color
		// colors[1]: R-value: rate of change to angle, other values not used
		extern const ParticleRecipe SmokeRecipe;

		// Fog particles emitter
		// default params: rate,size,ttl,dc
		// ground particle motion range: [min_dp.x, max_dp.x]
		// escape particle motion range: [min_dp.y, max_dp.y]
		// colors[0]: Fog color
		// colors[1].rg: min/max scalar coefficient of small particles 
		// colors[1].b: Emit ratio for small particles
		extern const ParticleRecipe FogRecipe;

		// Fountain particles emitter
		// default params: rate,size,ttl,dc
		// emit range: [min_dp.x,max_dp.x]
		// vertical motion: [min_dp.y, max_dp.y]
		// colors[0:4]: randomly selected
		extern const ParticleRecipe FountainRecipe;

		// Explosion particle emitter
		// default params: rate,size,ttl,dc
		// emit range: [min_dp.x, max_dp.x]
		// particle motion range: [min_dp.y, max_dp.y]
		// colors[0]: inner color (at min_dp.x)
		// colors[1]: outer color (at max_dp.x)
		extern const ParticleRecipe ExplosionRecipe;

		// Ground Explosion particle emitter
		// default params: rate,size,ttl,dc
		// emit range: [min_dp.x, max_dp.x]
		// particle motion range: [min_dp.y, max_dp.y]
		// colors[0]: inner color (at min_dp.x)
		// colors[1]: outer color (at max_dp.x)
		extern const ParticleRecipe GroundExplosionRecipe;

		extern const ParticleRecipe SnowRecipe;
		extern const ParticleRecipe SpiralRecipe;
		extern const ParticleRecipe BlastRecipe;
		extern const ParticleRecipe HelixRecipe;

		// Vortex particle emitter
		// default params: rate,size,ttl,dc
		// radius: radius of vortex
		// angular motion: [min_dp.x, max_dp.x]
		// vertical motion: [min_dp.y, max_dp.y]
		// colors[0:4]: randomly selected
		extern const ParticleRecipe VortexRecipe;
	}

}