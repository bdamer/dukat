#pragma once

#include <memory>
#include <dukat/dukat.h>

namespace dukat
{
	struct Sprite;

	class InputScene : public Scene2, public Controller
	{
	private:
		std::unique_ptr<Sprite> mask_sprite;
		std::array<std::unique_ptr<Sprite>, 8> button_sprites;
		std::array<std::unique_ptr<Sprite>, 4> debug_sprites;
		std::array<std::unique_ptr<Sprite>, 4> left_sprites;
		std::array<std::unique_ptr<Sprite>, 4> right_sprites;
		std::unique_ptr<Sprite> select_sprite;
		std::unique_ptr<Sprite> start_sprite;
		std::unique_ptr<Sprite> ltrigger_sprite;
		std::unique_ptr<Sprite> rtrigger_sprite;

		std::unique_ptr<TextMeshInstance> info_text;
		std::unique_ptr<TextMeshInstance> debug_text;

		Texture* button_on;
		Texture* button_off;

	public:
		InputScene(Game2* game2);
		~InputScene(void) { }

		void handle_keyboard(const SDL_Event& e);
		void update(float delta);
	};
}