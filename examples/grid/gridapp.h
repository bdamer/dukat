#pragma once

#include <memory>
#include <dukat/dukat.h>

namespace dukat
{
	class Player;

	class GridScene : public Scene, public Controller
	{
	private:
		static const int grid_size;
		static const float scale_factor; // terrain z-scale factor

		Game3* game;
		MeshGroup object_meshes;
		MeshGroup overlay_meshes;
		MeshInstance* info_mesh;
		MeshInstance* observer_mesh;
		std::unique_ptr<GridMesh> grid_mesh;
		std::unique_ptr<HeightMap> heightmap; // elevation data
		std::unique_ptr<OrbitalLight> light;
		
		Vector3 camera_target;
		
	public:
		GridScene(Game3* game);
		~GridScene(void) { }

		void update(float delta);
		void render(void);
		void handle_event(const SDL_Event& e);
		void handle_keyboard(const SDL_Event& e);
	};
}