#pragma once

#include <memory>
#include <dukat/dukat.h>

namespace dukat
{
	class Player;
	class Model3;

	class Game : public Game3
	{
	private:
		MeshGroup debug_meshes;
		std::unique_ptr<MeshGroup> object_meshes;
		MeshGroup overlay_meshes;
		MeshInstance* info_mesh;

		std::unique_ptr<Model3> model;
		int selected_mesh; // currently highlighted mesh 

		Vector3 camera_target;
		OrbitalLight light;
		bool enable_lighting;

		void init(void);
		void update(float delta);
		void update_debug_text(void);
		void render(void);
		void handle_event(const SDL_Event& e);
		void handle_keyboard(const SDL_Event& e);

	public:
		Game(Settings& settings) : Game3(settings), light(25.0f), enable_lighting(false), selected_mesh(-1) { }
		~Game(void) { }

		void load_model(const std::string& filename);
		void save_model(const std::string& filename);
		void toggle_debug(void);
	};
}