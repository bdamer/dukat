#pragma once

#include <memory>
#include <dukat/dukat.h>

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

	class MapgenScene : public Scene, public Controller
	{
	private:
		Game3* game;
		MeshGroup object_meshes;
		MeshGroup overlay_meshes;
		MeshInstance* fill_mesh;
		MeshInstance* line_mesh;
		MeshInstance* info_mesh;
		MapGraph graph;

		RenderMode render_mode;
		MapMode map_mode;
		int polygon_count;

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
		MapgenScene(Game3* game);
		~MapgenScene(void) { }

		void update(float delta);
		void render(void);
		bool handle_keyboard(const SDL_Event& e);
	};
}