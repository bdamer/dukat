#pragma once

#include <memory>
#include <dukat/game2.h>

namespace dukat
{
	struct Texture;
	class Sprite;
	class Surface;

	class Game : public Game2
	{
	private:
		const int texture_width = 256;
		const int texture_height = 256;

		std::unique_ptr<Texture> texture;
		std::unique_ptr<Surface> surface;
		std::unique_ptr<Sprite> sprite;
		std::unique_ptr<TextMeshInstance> info_text;
		std::unique_ptr<TextMeshInstance> debug_text;

		void init(void);
		void handle_keyboard(const SDL_Event& e);
		void update_debug_text(void);

		void test_load_image(void);
		void test_bw_image(void);
		void test_color_image(void);
		void update_texture(void);

	public:
		Game(const Settings& settings) : Game2(settings) { }
		~Game(void) { }
	};
}