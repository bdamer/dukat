#pragma once

#include <memory>
#include <dukat/game3.h>
#include <dukat/meshgroup.h>

namespace dukat
{
	class Environment;
	class Player;

	class Game : public Game3
	{
	private:
		MeshGroup debug_meshes;
		MeshGroup object_meshes;
		MeshGroup overlay_meshes;
		MeshInstance* ground_mesh;
		MeshInstance* skydome_mesh;
		MeshInstance* sun_mesh;
		MeshInstance* info_mesh;
		TextMeshInstance* mult_mesh;
		std::unique_ptr<Environment> env;
		std::unique_ptr<Texture> cubemap;

		void init(void);
		void init_environment(void);
		void handle_keyboard(const SDL_Event &e);
		void update(float delta);
		void update_debug_text(void);
		void render(void);

        int multiplier; // Simulation multiplier
		float total_time;

	public:
		Game(Settings& settings) : Game3(settings), multiplier(1), total_time(0.0f) { }
		~Game(void) { }

		void toggle_debug(void);
	};
}