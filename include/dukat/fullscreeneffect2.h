#pragma once

#include "color.h"

namespace dukat
{
	class Game2;
	class Animation;

    class FullscreenEffect2
    {
    private:
		static constexpr auto fade_layer = "fade_mask";
		static constexpr auto layer_priority = 999.0f;

		Game2* game;
		Animation* anim;
		Color color; // target color
		float alpha;

		void create_fade_layer(void);

    public:
		FullscreenEffect2(Game2* game);
      	~FullscreenEffect2(void);

		void fade_in(float duration, std::function<void(void)> callback = nullptr);
		void fade_out(float duration, std::function<void(void)> callback = nullptr);
		void set_color(Color color) { this->color = color; }
	};
}