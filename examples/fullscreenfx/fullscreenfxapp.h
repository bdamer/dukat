#pragma once

#include <memory>
#include <dukat/dukat.h>

namespace dukat
{
	struct Sprite;

	class FullscreenFXScene : public Scene2, public Controller
	{
	private:
		// Width of texture we are rendering to.
		static constexpr auto texture_width = 1024;
		std::unique_ptr<Sprite> sprite;
		std::unique_ptr<TextMeshInstance> info_text;
		std::unique_ptr<TextMeshInstance> debug_text;
		
		void init(void);
		void init_debug(void);

	public:
		FullscreenFXScene(Game2* game2);
		~FullscreenFXScene(void) { }

		void handle_keyboard(const SDL_Event& e);
	};
}