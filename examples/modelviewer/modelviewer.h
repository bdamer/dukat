#pragma once

#include <memory>
#include <dukat/dukat.h>

namespace dukat
{
	class Player;
	class Model3;

	class ModelviewerScene : public Scene, public Controller
	{
	private:
		Game3* game;
		std::unique_ptr<MeshGroup> object_meshes;
		MeshGroup overlay_meshes;
		MeshInstance* info_mesh;

		std::unique_ptr<Model3> model;
		int selected_mesh; // currently highlighted mesh 

		Vector3 camera_target;
		OrbitalLight light;
		bool enable_lighting;

	public:
		ModelviewerScene(Game3* game);
		~ModelviewerScene(void) { }

		void update(float delta);
		void render(void);
		bool handle_event(const SDL_Event& e);
		bool handle_keyboard(const SDL_Event& e);

		void load_model(const std::string& filename);
		void save_model(const std::string& filename);
	};
}