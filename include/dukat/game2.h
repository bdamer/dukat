#pragma once

#include <memory>
#include "gamebase.h"
#include "renderer2.h"
#include "scene2.h"
#include "fullscreeneffect2.h"

namespace dukat
{
	// Game implementation using the 2D subsystem.
	class Game2 : public GameBase
	{
	protected:
		std::unique_ptr<Renderer2> renderer;
		std::unique_ptr<FullscreenEffect2> effect;
		float speed_factor; // global speed factor used for updates
		virtual void update(float delta);
		void toggle_debug(void);
		void save_screenshot(const std::string& filename);

	public:
		Game2(Settings& settings);
		virtual ~Game2(void);
		std::unique_ptr<TextMeshInstance> create_text_mesh(void);
		std::unique_ptr<Sprite> create_sprite(const std::string& filename, Rect rect = { });
		float get_speed_factor(void) const { return speed_factor; }
		void set_speed_factor(float factor) { this->speed_factor = factor; }

		Renderer2* get_renderer(void) const { return renderer.get(); }
		FullscreenEffect2* get_effect(void) const { return effect.get(); }
	};
}