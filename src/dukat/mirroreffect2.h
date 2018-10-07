#pragma once

#include "effect2.h"

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
		std::string mirrored_layer;
		ShaderProgram* sprite_program;

	public:
		MirrorEffect2(ShaderProgram* sprite_program, const std::string& mirrored_layer)
			: sprite_program(sprite_program), mirrored_layer(mirrored_layer) { };
		~MirrorEffect2(void) { };

		void render(Renderer2* renderer, const AABB2& camera_bb);
	};
}