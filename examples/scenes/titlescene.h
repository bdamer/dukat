#pragma once

#include <dukat/dukat.h>
#include "textbutton.h"

namespace dukat
{
	class Game3;

	class TitleScene : public Scene, public Controller
	{
	private:
		Game3* game;
		MeshGroup overlay_meshes;
		std::unique_ptr<TextButton> start_button;
		std::unique_ptr<TextButton> options_button;
		std::unique_ptr<TextButton> quit_button;

	public:
		TitleScene(Game3* game);
		~TitleScene(void) { }

		void activate(void);
		void deactivate(void);
		void update(float delta);
		bool handle_keyboard(const SDL_Event& e);
		void render(void);
	};
}