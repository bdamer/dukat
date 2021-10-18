#pragma once

#include <memory>
#include <dukat/dukat.h>

namespace dukat
{
	class LightingApp : public Scene2, public Controller
	{
	private:
		static constexpr auto camera_height = 180;
		static constexpr auto num_lights = 4;

		std::unique_ptr<TextMeshInstance> info_text;
		std::unique_ptr<TextMeshInstance> debug_text;

		Texture* ground_texture;
		std::unique_ptr<Sprite> ground_sprite;
		std::array<std::unique_ptr<Sprite>, num_lights> light_sprites;

		void create_layers(void);
		void create_lights(void);

	public:
		LightingApp(Game2* game2);
		~LightingApp(void) { }

		void update(float delta);

		void handle_keyboard(const SDL_Event& e);
	};
}