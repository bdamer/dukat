#pragma once

#include "effect2.h"
#include "vector2.h"

namespace dukat
{
	class Game2;
	class ShaderProgram;

	// Effect to mirror another layer's sprites.
	// Allows to render them with a different shader, for example
	// for reflections.
	class MirrorEffect2 : public Effect2
	{
	private:
		ShaderProgram* sprite_program;
		ShaderProgram* particle_program;
		std::string mirrored_layer;
		// Bounding box shift
		Vector2 min_shift;
		Vector2 max_shift;

	public:
		MirrorEffect2(ShaderProgram* sprite_program, ShaderProgram* particle_program, const std::string& mirrored_layer, const Vector2& min_shift = { 0,0 }, const Vector2& max_shift = { 0,0 })
			: sprite_program(sprite_program), particle_program(particle_program), mirrored_layer(mirrored_layer), min_shift(min_shift), max_shift(max_shift) { };
		~MirrorEffect2(void) { };

		void render(Renderer2* renderer, const AABB2& camera_bb);
		void set_shift(const Vector2& min_shift, const Vector2& max_shift) { this->min_shift = min_shift; this->max_shift = max_shift; }
	};
}