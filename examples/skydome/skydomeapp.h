#pragma once

#include <memory>
#include <dukat/dukat.h>

namespace dukat
{
	class Environment;
	class Player;

	class SkydomeScene : public Scene, public Controller
	{
	private:
		Game3* game;
		MeshGroup object_meshes;
		MeshGroup overlay_meshes;
		MeshInstance* ground_mesh;
		MeshInstance* skydome_mesh;
		MeshInstance* sun_mesh;
		MeshInstance* info_mesh;
		TextMeshInstance* mult_mesh;
		std::unique_ptr<Environment> env;

        int multiplier; // Simulation multiplier
		float total_time;

		void init_environment(void);

	public:
		SkydomeScene(Game3* game);
		~SkydomeScene(void) { }

		bool handle_keyboard(const SDL_Event &e);
		void update(float delta);
		void render(void);
	};
}