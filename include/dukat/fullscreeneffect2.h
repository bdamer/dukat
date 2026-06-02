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
		struct ActiveEffect
		{
			int id;
			Color color;
			ShaderProgram* sp;
		};

		struct EffectState
		{
			Animation* anim;
			ShaderProgram* sp;
			Color color; // target color
			float alpha;
			std::vector<ActiveEffect> effect_registry;

			EffectState(void) : anim(nullptr), sp(nullptr), color({ 0.f, 0.f, 0.f, 0.f }), alpha(0.f) { }
		};

		Game2* game;
		EffectState state;
		// default composite binder 
		std::function<void(ShaderProgram* sp)> composite_binder;
		// Stashed effect states
		std::unordered_map<std::string, EffectState> state_stash;

    public:
		static constexpr auto default_vsh = "fx_default.vsh";
		static constexpr auto default_fsh = "fx_default.fsh";
		static constexpr auto solid_fsh = "fx_solid.fsh";

		FullscreenEffect2(Game2* game);
      	~FullscreenEffect2(void);

		void set_color(Color color) { state.color = color; }
		Color get_color(void) const { return state.color; }
		void set_alpha(float alpha) { state.alpha = alpha; }
		float get_alpha(void) const { return state.alpha; }
		void set_composite_program(ShaderProgram* sp, std::function<void(ShaderProgram*)> composite_binder = nullptr);
		void reset_composite_program(void);

		// Triggers timed fade-in / fade-out. This will replace the current shader function.
		void begin_fade_in(float duration, std::function<void(void)> callback = nullptr);
		void begin_fade_out(float duration, std::function<void(void)> callback = nullptr);

		// Adjusts alpha towards target value over duration
		void begin_anim(float duration, float target, std::function<void(void)> callback = nullptr);
		void cancel_anim(void);

		// Stashes and restores effect state under a given ID.
		void stash(const std::string& id);
		void restore(const std::string& id);

		// Named effect registry: tracks active overlay effects (e.g. modifier frame borders).
		// push cancels any running fade and shows the effect immediately; pop fades out when last.
		void push_effect(int id, const Color& color, ShaderProgram* sp);
		void pop_effect(int id);
		void clear_effects(void);
	};
}