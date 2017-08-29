#pragma once

#include <memory>
#include <dukat/game3.h>
#include <dukat/mapgraph.h>
#include <dukat/meshgroup.h>
#include <dukat/voronoi.h>

namespace dukat
{
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

		// Generates water / land mesh.
        void create_water_land_mesh(MeshData* mesh, float z_scale = 1.0f);
        // Generates mesh indicating elevation of each map cell. 
        void create_elevation_mesh(MeshData* mesh, float z_scale = 1.0f);
        // Generates mesh indicating moisture of each map cell.
        void create_moisture_mesh(MeshData* mesh, float z_scale = 1.0f);
        // Generates mesh to showcase biome of each map cell.
        void create_biomes_mesh(MeshData* mesh, float z_scale = 1.0f);
        // Generates mesh of polygon outlines
        void create_edge_mesh(MeshData* mesh, float z_scale = 1.0f);
        // Generates mesh of rivers
        void create_river_mesh(MeshData* mesh, float z_scale = 1.0f);

	public:
		Game(Settings& settings) : Game3(settings) { }
		~Game(void) { }

		void toggle_debug(void);
	};
}