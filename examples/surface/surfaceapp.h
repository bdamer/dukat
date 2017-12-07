#pragma once

#include <memory>
#include <dukat/dukat.h>

namespace dukat
{
	struct Texture;
	struct Sprite;
	class Surface;

	class SurfaceScene : public Scene2, public Controller
	{
	private:
		const int texture_width = 256;
		const int texture_height = 256;

		std::unique_ptr<Texture> texture;
		std::unique_ptr<Surface> surface;
		std::unique_ptr<Sprite> sprite;
		std::unique_ptr<TextMeshInstance> info_text;
		std::unique_ptr<TextMeshInstance> debug_text;

		void test_load_image(void);
		void test_bw_image(void);
		void test_color_image(void);
		void update_texture(void);

	public:
		SurfaceScene(Game2* game2);
		~SurfaceScene(void) { }

		void handle_keyboard(const SDL_Event& e);
	};
}