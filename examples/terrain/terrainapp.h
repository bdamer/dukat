#pragma once

#include <memory>
#include <vector>
#include <dukat/game3.h>
#include <dukat/meshgroup.h>
#include <dukat/texturecache.h>

namespace dukat
{
	class Player;
	class ClipMap;
	class HeightMap;
	struct Color;

	class Game : public Game3
	{
	private:
		int max_levels;
		int level_size;

		MeshGroup debug_meshes;
		MeshGroup object_meshes;
		MeshGroup overlay_meshes;
		MeshInstance* observer_mesh;
		MeshInstance* elevation_mesh;
		MeshInstance* info_mesh;

		// Show elevation or normal map level
		bool show_elevation_map;
		// texture to display level data
		std::unique_ptr<Texture> texture;
		// Which level to show
		int display_level;
		Material debug_mat;
		std::vector<Color> palette;

		bool first_person_camera;
		bool direct_camera_control;

		std::unique_ptr<ClipMap> clip_map;
		std::unique_ptr<HeightMap> height_map;

		void init(void);
		void handle_keyboard(const SDL_Event& e);
		void update(float delta);
		void update_debug_text(void);
		void render(void);

		void build_palette(void);
		void load_mtrainier(void);
		void load_pugetsound(void);
		void switch_to_first_person_camera(void);
		void switch_to_fixed_camera(void);

	public:
		Game(const Settings& settings) : Game3(settings) { }
		~Game(void) { }

		void toggle_debug(void);
	};
}