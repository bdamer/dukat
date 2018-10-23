#include "stdafx.h"
#include "gridmesh.h"

#include "blockbuilder.h"

namespace dukat
{
    static const std::string uniform_size = "u_size";
    static const std::string uniform_one_over_size = "u_one_over_size";
    static const std::string uniform_grid_scale = "u_grid_scale";

	GridMesh::GridMesh(Game3* game, int grid_size, float scale_factor)
		: MeshInstance(), game(game), grid_size(grid_size), scale_factor(scale_factor), 
		tile_spacing(1.0f), base_elevation(0.0f), texture_scale(1.0f / 16.0f)
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

        // Set up framebuffer for normal map generation (normal map is twice the size of height map)
        const auto nm_size = 2 * grid_size;
        auto fb_normal = std::make_unique<FrameBuffer>(nm_size, nm_size, true, false);
        normal_texture = fb_normal->texture.get();
        glBindTexture(GL_TEXTURE_2D, normal_texture->id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // 2-channel GL_RG16F texture for normal data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, nm_size, nm_size, 0, GL_RG, GL_FLOAT, nullptr);
		set_texture(normal_texture, 1);
        
        normal_pass = std::make_unique<EffectPass>();
        normal_pass->set_fbo(std::move(fb_normal));
        normal_pass->set_program(game->get_shaders()->get_program("fx_heightmap_normal.vsh", "fx_heightmap_normal.fsh"));
        normal_pass->set_texture(heightmap_texture.get(), 0);

        // size of normal map
        normal_pass->set_attribute(uniform_size, { static_cast<float>(nm_size) });
        // 1 / normal map texture size 
        const auto one_over_size = 1.0f / static_cast<float>(nm_size);
        normal_pass->set_attribute(uniform_one_over_size, { one_over_size });
        // pass in ratio of z to x/y grid spacing
        const auto grid_scale = -0.5f * scale_factor;
        normal_pass->set_attribute(uniform_grid_scale, { grid_scale, grid_scale });
	}

	void GridMesh::update(float delta)
	{
		// We're repurposing the model matrix to pass in information about the location and scale of the grid.
		// grid scale at current level
		transform.mat_model.m[0] = transform.mat_model.m[1] = tile_spacing;
		// origin of current block in world-space
		transform.mat_model.m[2] = transform.mat_model.m[3] = 0.0f;
		// 1 / texture width,height
		transform.mat_model.m[4] = transform.mat_model.m[5] = 1.0f / (float)grid_size;
		// Base elevation
		transform.mat_model.m[12] = base_elevation;
		// ZScale of height map 
		transform.mat_model.m[13] = scale_factor * tile_spacing;
		transform.mat_model.m[14] = texture_scale;
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

		update_normal_map();
	}

	void GridMesh::set_terrain_texture(std::unique_ptr<Texture> terrain_texture)
	{
		this->terrain_texture = std::move(terrain_texture);
		this->set_texture(this->terrain_texture.get(), 2);
	}
}