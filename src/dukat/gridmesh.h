#pragma once

#include "heightmap.h"
#include "meshinstance.h"
#include "game3.h"

namespace dukat
{
	class GridMesh : public MeshInstance
	{
	private:
		std::unique_ptr<Texture> heightmap_texture; // 1-channel GL_R32F texture used for elevation data.
		std::unique_ptr<Texture> terrain_texture; // RGB texture array for texture splatting.
		std::unique_ptr<MeshData> mesh_data; // n*n grid mesh

	public:
		const int grid_size;
		int tile_spacing; // size of grid tiles
		float base_elevation;
		float scale_factor;

		GridMesh(Game3* game, int grid_size, float scale_factor);
		virtual ~GridMesh(void) { };

		// Updates mesh transform.
		void update(float delta);

		void load_height_level(const HeightMap::Level& level);
		void set_terrain_texture(std::unique_ptr<Texture> terrain_texture);
	};
}