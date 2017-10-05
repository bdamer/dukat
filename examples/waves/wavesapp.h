#pragma once

#include <memory>
#include <dukat/dukat.h>

namespace dukat
{
	class WaveMesh;
	class Environment;

	class WaveScene : public Scene, public Controller
	{
	private:
		Game3* game;
		MeshGroup object_meshes;
		MeshGroup overlay_meshes;
		MeshInstance* info_mesh;
		MeshInstance* quad_mesh;
		MeshInstance* skydome_mesh;
		std::unique_ptr<WaveMesh> wave_mesh;
		std::unique_ptr<Environment> env;

		Vector3 camera_target;
		bool wireframe; // flag to render wireframe mesh

		void init_environment(void);

	public:
		WaveScene(Game3* game);
		~WaveScene(void) { }

		void update(float delta);
		void render(void);
		void handle_event(const SDL_Event& e);
		void handle_keyboard(const SDL_Event& e);
	};
}