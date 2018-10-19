#pragma once

#include <memory>
#include <dukat/dukat.h>

namespace dukat
{
	class HelloDukat : public Scene2, public Controller
	{
	private:
		const int camera_width = 320;
		const int camera_height = 240;

		std::unique_ptr<TextMeshInstance> info_text;
		std::unique_ptr<TextMeshInstance> debug_text;

	public:
		HelloDukat(Game2* game2);
		~HelloDukat(void) { }

		void handle_keyboard(const SDL_Event& e);
	};
}