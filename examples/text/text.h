#pragma once

#include <memory>
#include <dukat/dukat.h>

namespace dukat
{
	class TextScene : public Scene2, public Controller
	{
	private:
		static constexpr auto layer_name = "main";

		int font_size;
		std::unique_ptr<TextMeshInstance> center_text;
		std::unique_ptr<TextMeshInstance> info_text;
		std::unique_ptr<TextMeshInstance> debug_text;

		void reset_text(void);

	public:
		TextScene(Game2* game);
		~TextScene(void) { }

		void update(float delta);
		void handle_keyboard(const SDL_Event& e);
	};
}