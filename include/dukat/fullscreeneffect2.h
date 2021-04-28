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

		// default composite binder 
		std::function<void(ShaderProgram* sp)> composite_binder;

    public:
		FullscreenEffect2(Game2* game);
      	~FullscreenEffect2(void);

		void set_color(Color color) { this->color = color; }
		Color get_color(void) const { return color; }
		void set_alpha(float alpha) { this->alpha = alpha; }
		float get_alpha(void) const { return alpha; }
		void set_composite_program(ShaderProgram* sp, std::function<void(ShaderProgram*)> composite_binder = nullptr);
		void reset_composite_program(void);
		// Restores last composite program used.
		void restore_composite_program(void);

		// Triggers timed fade-in / fade-out. This will replace the current shader function.
		void begin_fade_in(float duration, std::function<void(void)> callback = nullptr);
		void begin_fade_out(float duration, std::function<void(void)> callback = nullptr);

		// Adjusts alpha towards target value over duration
		void begin_anim(float duration, float target, std::function<void(void)> callback = nullptr);
		void cancel_anim(void);
	};
}