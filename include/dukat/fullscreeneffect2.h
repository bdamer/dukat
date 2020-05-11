#pragma once

#include "color.h"

namespace dukat
{
	class Game2;
	class Animation;
	class ShaderProgram;

    class FullscreenEffect2
    {
    private:
		static constexpr auto default_vsh = "fx_default.vsh";
		static constexpr auto default_fsh = "fx_default.fsh";

		Game2* game;
		Animation* anim;
		ShaderProgram* last_sp;
		Color color; // target color
		float alpha;

    public:
		FullscreenEffect2(Game2* game);
      	~FullscreenEffect2(void);

		void fade_in(float duration, std::function<void(void)> callback = nullptr);
		void fade_out(float duration, std::function<void(void)> callback = nullptr);
		void set_color(Color color) { this->color = color; }
		void set_composite_program(ShaderProgram* sp, std::function<void(ShaderProgram*)> composite_binder = nullptr);
		void reset_composite_program(void);
		// Restores last composite program used.
		void restore_composite_program(void);
	};
}