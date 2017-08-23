#include "stdafx.h"
#include "gridmesh.h"

#include "blockbuilder.h"

namespace dukat
{
	GridMesh::GridMesh(Game3* game, int grid_size, float scale_factor)
		: MeshInstance(), tile_spacing(2), grid_size(grid_size), base_elevation(0.0f), scale_factor(scale_factor)
	{ 
		// Create elevation texture
		heightmap_texture = std::make_unique<Texture>(grid_size, grid_size, ProfileLinear);
		glBindTexture(GL_TEXTURE_2D, heightmap_texture->id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Generate mesh for terrain grid
		BlockBuilder bb;
		bb.add_block(grid_size, grid_size);
		mesh_data = bb.create_mesh();
		set_mesh(mesh_data.get());
		set_program(game->get_shaders()->get_program("sc_heightmap.vsh", "sc_heightmap.fsh"));
		set_texture(heightmap_texture.get(), 0);
	}

	void GridMesh::update(float delta)
	{
		// We're repurposing the model matrix to pass in information about the location and scale of the grid.
		// grid scale at current level
		transform.mat_model.m[0] = transform.mat_model.m[1] = (float)tile_spacing;
		// origin of current block in world-space
		transform.mat_model.m[2] = transform.mat_model.m[3] = 0.0f;
		// 1 / texture width,height
		transform.mat_model.m[4] = transform.mat_model.m[5] = 1.0f / (float)grid_size;
		// Base elevation
		transform.mat_model.m[12] = base_elevation;
		// ZScale of height map 
		transform.mat_model.m[13] = scale_factor * (float)tile_spacing;
	}

	void GridMesh::load_height_level(const HeightMap::Level& level)
	{
		glBindTexture(GL_TEXTURE_2D, heightmap_texture->id);
#if OPENGL_VERSION >= 30
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, grid_size, grid_size, 0, GL_RED, GL_FLOAT, level.data.data());
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, grid_size, grid_size, 0, GL_RED, GL_FLOAT, level.data.data());
#endif
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void GridMesh::set_terrain_texture(std::unique_ptr<Texture> terrain_texture)
	{
		this->terrain_texture = std::move(terrain_texture);
		this->set_texture(this->terrain_texture.get(), 1);
	}
}