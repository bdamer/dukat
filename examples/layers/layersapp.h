#pragma once

#include <memory>
#include <dukat/dukat.h>

namespace dukat
{
	struct Texture;
	struct Sprite;
	class Surface;

	class LayersScene : public Scene2, public Controller
	{
	private:
		// virtual width of game window in pixels. height is determined at runtime based 
		// on aspect ratio of actual window.
		static constexpr int game_width = 160;

		std::unique_ptr<TextMeshInstance> info_text;
		std::unique_ptr<TextMeshInstance> debug_text;

		Texture* mask_texture;
		Texture* ground_texture;
		std::unique_ptr<Sprite> ground_sprite;
		std::unique_ptr<Sprite> player_sprite;
		std::unique_ptr<Sprite> barrel_sprite;

	public:
		LayersScene(Game2* game2);
		~LayersScene(void) { }

		void handle_keyboard(const SDL_Event& e);
		void update(float delta);
		void resize(int width, int height);
	};
}