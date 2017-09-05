#pragma once

#include <memory>
#include <dukat/dukat.h>

namespace dukat
{
	class Player;

	class Game : public Game3
	{
	private:
		MeshGroup debug_meshes;
		MeshGroup object_meshes;
		MeshGroup overlay_meshes;
		MeshInstance* info_mesh;

		OrbitalLight light;
		bool animate_light;

		void init(void);
		void update(float delta);
		void update_debug_text(void);
		void render(void);
		void handle_event(const SDL_Event& e);
		void handle_keyboard(const SDL_Event& e);

	public:
		Game(Settings& settings) : Game3(settings), light(10.0f), animate_light(true) { }
		~Game(void) { }

		void toggle_debug(void);
	};
}