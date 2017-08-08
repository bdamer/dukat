#pragma once

#include <memory>
#include <dukat/game3.h>
#include <dukat/meshgroup.h>

namespace dukat
{
	class WaveMesh;
	class Environment;

	class Game : public Game3
	{
	private:
		static constexpr int grid_size = 128;
		
		MeshGroup debug_meshes;
		MeshGroup object_meshes;
		MeshGroup overlay_meshes;
		MeshInstance* info_mesh;
		MeshInstance* quad_mesh;
		MeshInstance* skydome_mesh;
		std::unique_ptr<WaveMesh> wave_mesh;
		std::unique_ptr<Environment> env;

		Vector3 camera_target;
		bool wireframe; // flag to render wireframe mesh

		void init(void);
		void init_environment(void);
		void update(float delta);
		void update_debug_text(void);
		void render(void);
		void handle_event(const SDL_Event& e);
		void handle_keyboard(const SDL_Event& e);

	public:
		Game(Settings& settings) : Game3(settings) { }
		~Game(void) { }

		void toggle_debug(void);
	};
}