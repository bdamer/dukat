#pragma once

#include <memory>
#include <dukat/game3.h>
#include <dukat/meshgroup.h>
#include "voronoi.h"
#include "mapgraph.h"

namespace dukat
{
	class Player;

	enum RenderMode
	{
		Overhead,
		Perspective
	};

	enum MapMode
	{
		LandWater,
		Elevation,
		Moisture,
		Biomes
	};

	class Game : public Game3
	{
	private:
		MeshGroup debug_meshes;
		MeshGroup object_meshes;
		MeshGroup overlay_meshes;
		MeshInstance* fill_mesh;
		MeshInstance* line_mesh;
		MeshInstance* info_mesh;
		MapGraph graph;

		RenderMode render_mode;
		MapMode map_mode;
		int polygon_count;

		void init(void);
		void update(float delta);
		void update_debug_text(void);
		void render(void);
		void handle_keyboard(const SDL_Event& e);

		void switch_mode(void);
		void generate_map(void);

	public:
		Game(Settings& settings) : Game3(settings) { }
		~Game(void) { }

		void toggle_debug(void);
	};
}