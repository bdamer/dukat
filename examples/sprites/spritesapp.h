#pragma once

#include <memory>
#include <queue>
#include <dukat/dukat.h>

namespace dukat
{
	struct Sprite;

	class SpritesScene : public Scene2, public Controller
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

	public:
		SpritesScene(Game2* game);
		~SpritesScene(void) { }

		void handle_keyboard(const SDL_Event& e);
		void update(float delta);
	};
}