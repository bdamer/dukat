#pragma once

#include "effect2.h"

namespace dukat
{
	class Game2;
	class ShaderProgram;

	// Effect to render shadow below this layer's sprites.
	class ShadowEffect2 : public Effect2
	{
	private:
		ShaderProgram* sprite_program;
		float radius;
		float alpha;

	public:
		ShadowEffect2(ShaderProgram* sprite_program)
			: sprite_program(sprite_program), radius(0.25f), alpha(0.7f) { };
		~ShadowEffect2(void) { };

		void set_radius(float radius) { this->radius = radius; }
		void set_alpha(float alpha) { this->alpha = alpha; }
		void render(Renderer2* renderer, const AABB2& camera_bb);
	};
}