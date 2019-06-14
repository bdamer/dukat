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
		Vector2 shift;

	public:
		MirrorEffect2(ShaderProgram* sprite_program, ShaderProgram* particle_program, const std::string& mirrored_layer, const Vector2& shift = { 0,0 })
			: sprite_program(sprite_program), particle_program(particle_program), mirrored_layer(mirrored_layer), shift(shift) { };
		~MirrorEffect2(void) { };

		void render(Renderer2* renderer, const AABB2& camera_bb);
		void set_shift(const Vector2& shift) { this->shift = shift; }
	};
}