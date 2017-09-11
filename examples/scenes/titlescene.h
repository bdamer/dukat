#pragma once

#include <dukat/controller.h>
#include <dukat/scene.h>
#include <dukat/meshgroup.h>

namespace dukat
{
	class Game3;

	class TitleScene : public Scene, public Controller
	{
	private:
		Game3* game;
		MeshGroup overlay_meshes;

	public:
		TitleScene(Game3* game);
		~TitleScene(void) { }

		void activate(void);
		void update(float delta);
		bool handle_keyboard(const SDL_Event& e);
		void render(void);
	};
}