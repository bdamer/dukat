#pragma once

#include <memory>
#include <queue>
#include <dukat/game2.h>
#include <dukat/particle.h>

namespace dukat
{
	class Sprite;

	class Game : public Game2
	{
	private:
		static const int window_width = 400;
		static const int window_height = 300;

		std::unique_ptr<Sprite> bg_sprite;
		std::unique_ptr<Sprite> sprite;		
		RenderLayer2* bg_layer;
		RenderLayer2* particle_layer;
		RenderLayer2* main_layer;
		std::unique_ptr<TextMeshInstance> info_text;
		std::unique_ptr<TextMeshInstance> debug_text;
		Vector2 sprite_vel;
		bool particles_enabled;

		void init(void);
		void handle_keyboard(const SDL_Event& e);
		void update_debug_text(void);
		void update(float delta);

	public:
		Game(Settings& settings) : Game2(settings), particles_enabled(true) { }
		~Game(void) { }
	};
}