#pragma once

#ifndef OPENGL_VERSION
#include "version.h"
#endif // !OPENGL_VERSION

#include "effectpass.h"
#include "heightmap.h"
#include "meshinstance.h"
#include "game3.h"

namespace dukat
{
	class GridMesh : public MeshInstance
	{
	private:
		Game3* game;
		std::unique_ptr<Texture> heightmap_texture; // 1-channel GL_R32F texture used for elevation data.
		std::unique_ptr<Texture> terrain_texture; // RGB texture array for texture splatting.
		std::unique_ptr<MeshData> mesh_data; // n*n grid mesh
        std::unique_ptr<EffectPass> normal_pass; // generates normal map
        Texture* normal_texture; // normal map
		
	public:
		const int grid_size; // width / length of grid
		const float scale_factor; // elevation scale factor
		float tile_spacing; // size of grid tiles
		float base_elevation; // base elevation
		float texture_scale; 

		GridMesh(Game3* game, int grid_size, float scale_factor);
		virtual ~GridMesh(void) { };

		// Updates mesh transform.
		virtual void update(float delta);
		void update_normal_map(void) { normal_pass->render(game->get_renderer()); }

		void load_height_level(const HeightMap::Level& level);
		void set_terrain_texture(std::unique_ptr<Texture> terrain_texture);
		Texture* get_heightmap_texture(void) const { return heightmap_texture.get(); }
		Texture* get_normal_texture(void) const { return normal_texture; }
	};
}