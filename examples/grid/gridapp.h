#pragma once

#include <memory>
#include <dukat/game3.h>
#include <dukat/heightmap.h>
#include <dukat/meshgroup.h>

namespace dukat
{
	class Player;

	class Game : public Game3
	{
	private:
		const int grid_size = 64;
		MeshGroup debug_meshes;
		MeshGroup object_meshes;
		MeshGroup overlay_meshes;
		MeshInstance* info_mesh;
		MeshInstance* observer_mesh;
		MeshInstance* grid_mesh;

		std::unique_ptr<HeightMap> heightmap; // elevation data
		std::unique_ptr<Texture> heightmap_texture; // 1-channel GL_R32F texture used for elevation data.
		std::unique_ptr<Texture> terrain_texture; // RGB texture array for texture splatting.

		Vector3 camera_target;

		bool wireframe; // flag to render wireframe mesh
		int tile_spacing = 1;

		void init(void);
		void update(float delta);
		void update_debug_text(void);
		void render(void);
		void handle_event(const SDL_Event& e);
		void handle_keyboard(const SDL_Event& e);

	public:
		Game(Settings& settings) : Game3(settings), wireframe(false) { }
		~Game(void) { }

		void toggle_debug(void);
	};
}