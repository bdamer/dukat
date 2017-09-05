#pragma once

#include <memory>
#include <vector>
#include <dukat/dukat.h>

namespace dukat
{
	class Player;
	class ClipMap;
	class HeightMap;
	struct Color;

	enum CameraMode
	{
		Terrain = 0,
		FirstPerson = 1,
		Overhead = 2
	};

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
		MeshInstance* cursor_mesh;

		// Show elevation or normal map level
		bool show_elevation_map;
		// texture to display level data
		std::unique_ptr<Texture> texture;
		// Which level to show
		int display_level;
		Material debug_mat;
		std::vector<Color> palette;

		CameraMode camera_mode;
		bool direct_camera_control;

		std::unique_ptr<ClipMap> clip_map;
		std::unique_ptr<HeightMap> height_map;

		void init(void);
		void handle_event(const SDL_Event& e);
		void handle_keyboard(const SDL_Event& e);

		void update(float delta);
		void update_debug_text(void);
		void render(void);

		void build_palette(void);
		void load_mtrainier(void);
		void load_pugetsound(void);
		void load_blank(void);
		void generate_terrain(void);
		void switch_camera_mode(CameraMode mode);

	public:
		Game(Settings& settings) : Game3(settings) { }
		~Game(void) { }

		void toggle_debug(void);
	};
}