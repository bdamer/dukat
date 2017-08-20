#include "stdafx.h"
#include "heatmap.h"

#include <dukat/blockbuilder.h>
#include <dukat/log.h>
#include <dukat/mathutil.h>
#include <dukat/meshbuilder2.h>
#include <dukat/renderer.h>
#include <dukat/shape.h>
#include <dukat/sysutil.h>
#include <dukat/vertextypes3.h>

#include <set>

namespace dukat
{
    static const std::string uniform_size = "u_size";
    static const std::string uniform_one_over_size = "u_one_over_size";
    static const std::string uniform_grid_scale = "u_grid_scale";
    const float HeatMap::dissipation_rate = 0.005f;
    const float HeatMap::transfer_factor = 0.25f;
    const float HeatMap::elevation_increase = 0.05f;
    const float HeatMap::growth_rate = 0.0025f;
    const float HeatMap::burn_rate = 0.01f;
    const float HeatMap::transmission_threshold = 0.01f;
    const float HeatMap::vegetation_threshold = 0.02f;
    const float HeatMap::min_emission = 5.0f;
    const float HeatMap::max_emission = 10.0f;
    const float HeatMap::min_period = 120.0f;
    const float HeatMap::max_period = 360.0f;

    HeatMap::HeatMap(Game3* game, int map_size) : game(game), map_size(map_size), 
        cells(map_size * map_size), tile_spacing(1)
    {
        heightmap = std::make_unique<HeightMap>(1);
        heightmap->allocate(map_size);

        // Create elevation texture
        heightmap_texture = std::make_unique<Texture>(map_size, map_size, ProfileNearest);
        glBindTexture(GL_TEXTURE_2D, heightmap_texture->id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, map_size, map_size, 0, GL_RED, GL_FLOAT, nullptr);
        
        // Create heatmap texture
        heatmap_texture = std::make_unique<Texture>(map_size, map_size, ProfileNearest);
        glBindTexture(GL_TEXTURE_2D, heatmap_texture->id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, map_size, map_size, 0, GL_RGB, GL_FLOAT, nullptr);

        // Set up framebuffer for normal map generation (normal map is twice the size of height map)
		normal_program = game->get_shaders()->get_program("fx_heatmap_normal.vsh", "fx_heatmap_normal.fsh");
		fb_normal = std::make_unique<FrameBuffer>(2 * map_size, 2 * map_size, true, false);
        glBindTexture(GL_TEXTURE_2D, fb_normal->texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // 2-channel GL_RG16F texture for normal data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 2 * map_size, 2 * map_size, 0, GL_RG, GL_FLOAT, nullptr);

		MeshBuilder2 builder;
		quad_normal = builder.build_textured_quad();

        // Generate mesh for terrain grid
        program = game->get_shaders()->get_program("sc_heatmap.vsh", "sc_heatmap.fsh");
		BlockBuilder bb;
		bb.add_block(map_size, map_size);
        grid_mesh = bb.create_mesh();

        // Generate array containing textures used for splatting
		int texture_size = 1024;
		terrain_texture = std::make_unique<Texture>(texture_size, texture_size);
		terrain_texture->target = GL_TEXTURE_2D_ARRAY;
		glBindTexture(GL_TEXTURE_2D_ARRAY, terrain_texture->id);
		// TODO: anisotropic
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, texture_size, texture_size, 4,
			0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, nullptr);

		int i = 0;
		auto sand_surface = game->get_textures()->load_surface("sand01_1024.png");
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i++, texture_size, texture_size, 1,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, sand_surface->get_surface()->pixels);
		auto grass_surface = game->get_textures()->load_surface("grass01_1024.png");
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i++, texture_size, texture_size, 1,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, grass_surface->get_surface()->pixels);
		auto dirt_surface = game->get_textures()->load_surface("dirt01_1024.png");
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i++, texture_size, texture_size, 1,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, dirt_surface->get_surface()->pixels);
		auto rock_surface = game->get_textures()->load_surface("rock01_1024.png");
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i++, texture_size, texture_size, 1,
			GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, rock_surface->get_surface()->pixels);

		glGenerateMipmap(GL_TEXTURE_2D_ARRAY);        
    }

    void HeatMap::reset(void)
    {
        for (auto c : cells)
        {
            c.t = c.v = c.delta = 0.0f;
        }
    }

    void HeatMap::load(const std::string& filename)
	{
        heightmap->load(filename);

        // TODO: restore heatmap state + emitters
	}

	void HeatMap::save(const std::string& filename) const
	{
        heightmap->save(filename);

        // TODO: store heatmap state + emitters
	}

	void HeatMap::generate(const HeightMapGenerator& gen)
	{
	    gen.generate(heightmap->get_level(0));
	}

    void HeatMap::add_emitters(int num_emitters, int radius)
    {
        CircleShape shape(0.75f);

        // normalize radius
        auto hs = (float)(map_size / 2);
        auto rn = (float)radius / hs;

        // build up a set of occupied emitter positions
        std::set<int> occupied;
        for (const auto& e : emitters) 
        {
            occupied.insert(e.y * map_size + e.x);
        }

        // normalized center of cluster
        auto center = Vector2::random({-0.25f, -0.25f}, {0.25f, 0.25f});
        for (int j = 0; j < num_emitters; j++) 
        {
            int x, y;
            bool done = false;
            while (!done)
            {
                auto pos = center + Vector2::random({-rn, -rn}, {rn, rn});
                if (!shape.contains(pos))
                    continue;
                x = (int)std::floor((pos.x + 1.0f) * hs);
                y = (int)std::floor((pos.y + 1.0f) * hs);
                auto idx = y * map_size + x;
                if (occupied.count(idx) > 0)
                    continue;
                occupied.insert(idx);
                done = true;
            }

            Emitter e{ x, y };
            e.phase = randf(0.0f, two_pi);
            e.period = randf(min_period, max_period);
            e.max_emission = randf(min_emission, max_emission);
            emitters.push_back(e);
        }
    }

    // Computes output from each active emitter
    void HeatMap::emitter_phase(float delta)
    {
        for (auto& emitter : emitters)
        {
            auto idx = emitter.y * map_size + emitter.x;
            auto& cell = cells[idx];
            if (emitter.active)
            {
                emitter.phase += delta;
                // TODO: use lookup table
                auto factor = std::max(0.0f, std::cos(emitter.phase / emitter.period));
                cell.t += factor * emitter.max_emission * delta;
            }
        }
    }

    // Computes heat transmission between cells
    void HeatMap::compute_phase(float delta)
    {
        // Neighbors of current cell in clockwise order
        std::array<int,8> neighbors;
        
        const auto& level = heightmap->get_level(0);
        for (int i = 0; i < (int)cells.size(); i++)
        {
            auto& cell = cells[i];

            if (cell.t > transmission_threshold)
            {
                auto total_transfer = 0.0f;
                const auto x = i % map_size;
                const auto y = i / map_size;

                neighbors[0] = (y > 0) ? i - map_size : -1;
                neighbors[1] = (x < map_size - 1 && y > 0) ? i - map_size + 1 : -1;
                neighbors[2] = (x < map_size - 1) ? i + 1 : -1;
                neighbors[3] = (x < map_size - 1 && y < map_size - 1) ? i + map_size + 1 : -1;
                neighbors[4] = (y < map_size - 1) ? i + map_size : -1;
                neighbors[5] = (x > 0 && y < map_size - 1) ? i + map_size - 1 : -1;
                neighbors[6] = (x > 0) ? i - 1 : -1;
                neighbors[7] = (x > 0 && y > 0) ? i - map_size - 1 : -1;

                const auto transfer_rate = delta * cell.t / 8.0f;
                for (auto ni : neighbors)
                {
                    auto slope = ni >= 0 ? level.data[i] - level.data[ni] : level.data[i];
                    if (slope > 0.0f)
                    {
                        auto transfer = (1.0f - transfer_factor + transfer_factor * slope) * transfer_rate;
                        if (ni >= 0)
                        {
                            cells[ni].delta += transfer;
                        }
                        total_transfer += transfer;
                    }
                }

                cell.delta -= total_transfer;
            }
        }
    }

    // Applies temperature delta and increases elevations
    void HeatMap::update_phase(float delta)
    {
        auto& level = heightmap->get_level(0);
        for (auto i = 0u; i < cells.size(); i++)
        {
            auto& cell = cells[i];
            cell.t += cell.delta;
            cell.delta = 0.0f;

            // dissipation loss leads to elevation increase
            auto loss = std::min(cell.t, dissipation_rate * delta) ;
            cell.t -= loss;

            // limit growth as elevation increases - this avoids hills becoming
            // too "spiky"
            level[i] += (1.0f - level[i]) * loss * elevation_increase;

            if (cell.t >= vegetation_threshold) 
            {
                cell.v -= burn_rate * delta;
            }
            else
            {
                cell.v += growth_rate * delta;
            }

            // Clamp temperature at an upper value to avoid runaway heating
            // in the shader this value will once again be clamped between 0..1
            clamp(cell.t, 0.0f, 10.0f);
            clamp(cell.v, 0.0f, 1.0f);
            clamp(level[i], 0.0f, 1.0f);
        }
    }
    
    void HeatMap::update(float delta)
    {
        emitter_phase(delta);
        compute_phase(delta);
        update_phase(delta);
        update_textures();
    }

    void HeatMap::update_textures(void)
    {
        // Update heatmap texture
        heatmap_texture->bind(0);
        // TODO: review preferred input format
        // TODO: test using glTexSubImage and keep track of changed rect
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, map_size, map_size, 0, GL_RGB, GL_FLOAT, cells.data());

        // Update elevation map
        heightmap_texture->bind(0);
        auto& level = heightmap->get_level(0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, map_size, map_size, 0, GL_RED, GL_FLOAT, level.data.data());

        // Update normal maps

        // Disable blending so we can use ALPHA channel for map flags
        glDisable(GL_BLEND);

        fb_normal->bind();
        game->get_renderer()->switch_shader(normal_program);
        heightmap_texture->bind(0, normal_program);

        // size of normal map
        glUniform1f(normal_program->attr(uniform_size), (float)(2 * map_size));
        // 1 / normal map texture size 
        const auto one_over_size = 1.0f / (float)(2 * map_size);
        glUniform1f(normal_program->attr(uniform_one_over_size), one_over_size);
        // pass in ratio of z to x/y grid spacing
        const auto grid_scale = -0.5f * heightmap->get_scale_factor();
        glUniform2f(normal_program->attr(uniform_grid_scale), grid_scale, grid_scale);

        quad_normal->render(normal_program);

        fb_normal->unbind();
        glEnable(GL_BLEND);
        perfc.inc(PerformanceCounter::FRAME_BUFFERS);
    }

    void HeatMap::render(Renderer* renderer)
    {
        renderer->switch_shader(program);
        
        // bind elevation map
        heightmap_texture->bind(0, program);
        // bind normal map
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, fb_normal->texture);
        glUniform1i(program->attr("u_tex1"), 1);
        heatmap_texture->bind(2, program);
        terrain_texture->bind(3, program);

        // We're repurposing the model matrix to pass in information about the location and scale of the grid.
        Matrix4 model;
        // grid scale at current level
        model.m[0] = model.m[1] = (float)tile_spacing;
        // origin of current block in world-space
        model.m[2] = model.m[3] = 0.0f;
        // 1 / texture width,height
        model.m[4] = model.m[5] = 1.0f / (float)map_size;
        // ZScale of height map 
        model.m[13] = heightmap->get_scale_factor() * (float)tile_spacing; 
        glUniformMatrix4fv(program->attr(Renderer::uf_model), 1, false, model.m);

        grid_mesh->render(program);

        perfc.inc(PerformanceCounter::MESHES);

        // unbind textures
        heatmap_texture->unbind();
		glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        heightmap_texture->unbind();
    }

    void HeatMap::toggle_emitters(void)
    {
        for (auto& e : emitters)
        {
            e.active = !e.active;       
        }
    }
}