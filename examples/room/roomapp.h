#pragma once

#include <memory>
#include <dukat/game3.h>
#include <dukat/meshgroup.h>

namespace dukat
{
	class Player;

	class Game : public Game3
	{
	private:
		MeshGroup debug_meshes;
		MeshGroup object_meshes;
		
		void init(void);
		void handle_keyboard(const SDL_Event& e);
		void update(float delta);
		void update_debug_text(void);
		void render(void);

	public:
		Game(const Settings& settings) : Game3(settings) { }
		~Game(void) { }

		void toggle_debug(void);
	};
}