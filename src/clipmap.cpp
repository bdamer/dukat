#include "stdafx.h"
#include <dukat/clipmap.h>
#include <dukat/blockbuilder.h>
#include <dukat/heightmap.h>
#include <dukat/log.h>
#include <dukat/mathutil.h>
#include <dukat/meshdata.h>
#include <dukat/meshbuilder2.h>
#include <dukat/perfcounter.h>
#include <dukat/plane.h>
#include <dukat/renderer.h>
#include <dukat/shadercache.h>
#include <dukat/vertextypes3.h>

namespace dukat
{
    // Uniform constants
    static const std::string uniform_size = "u_size";
    static const std::string uniform_one_over_size = "u_one_over_size";
    static const std::string uniform_tex_offset = "u_texture_offset";
    static const std::string uniform_rect = "u_rect";

	void ClipMapLevel::translate(const Vector2& offset)
	{
        last_shift = offset / scale;
		origin += offset;
		for (auto& bb : bounding_boxes)
		{
			bb.min.x += offset.x;
			bb.min.z += offset.y;
			bb.max.x += offset.x;
			bb.max.z += offset.y;
		}
	}

    ClipMap::ClipMap(Game3* game, int num_levels, int level_size, HeightMap* height_map)
        : num_levels(num_levels), level_size(level_size), texture_size(level_size + 1), game(game),
          height_map(height_map), min_level(0),
		  culling(true), stitching(true), blending(true), lighting(true)
    {
        log->debug("Creating new clipmap: {}x{}x{}", level_size, level_size, num_levels);
        // TODO: validate that level_size is a power of 2 - 1
        assert(level_size >= 7); // minimum allowed level size
        assert(level_size < 1024); // using 16 bit indeces

        build_buffers();
		build_levels();

        // Allocate buffer space
        buffer.resize(texture_size * texture_size);

        // Generate elevation map array.
        elevation_maps = std::make_unique<Texture>(texture_size, texture_size);
        elevation_maps->target = GL_TEXTURE_2D_ARRAY;
        glBindTexture(GL_TEXTURE_2D_ARRAY, elevation_maps->id);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_R32F, texture_size, texture_size, num_levels,
            0, GL_RED, GL_FLOAT, nullptr);

        // Set up framebuffer for elevation map update
        update_program = game->get_shaders()->get_program("fx_clipmap_update.vsh", "fx_clipmap_update.fsh");
        fb_update = std::make_unique<FrameBuffer>(texture_size, texture_size, false, false);

        // Set up texture used for elevation map updates
        update_texture = std::make_unique<Texture>(texture_size, texture_size);
        glBindTexture(GL_TEXTURE_2D, update_texture->id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, texture_size, texture_size, 0, GL_RED, GL_FLOAT, 0);

        // Generate normal map array 
        normal_maps = std::make_unique<Texture>(2 * texture_size, 2 * texture_size);
        normal_maps->target = GL_TEXTURE_2D_ARRAY;
        glBindTexture(GL_TEXTURE_2D_ARRAY, normal_maps->id);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RG16F, 2 * texture_size, 2 * texture_size, num_levels,
            0, GL_RG, GL_FLOAT, nullptr);

        // Set up framebuffer for normal map generation
		normal_program = game->get_shaders()->get_program("fx_clipmap_normal.vsh", "fx_clipmap_normal.fsh");
		fb_normal = std::make_unique<FrameBuffer>(2 * texture_size, 2 * texture_size, false, false);
		MeshBuilder2 builder;
		quad_normal = builder.build_textured_quad();

        // build quad for elevation map updates
		std::vector<VertexAttribute> attr;
		attr.push_back(VertexAttribute(Renderer::at_pos, 3));
		attr.push_back(VertexAttribute(Renderer::at_texcoord, 2));
		Vertex3PT verts[4] = {
			{ 0.0f,  1.0f, 0.0f, 0.0f, 1.0f }, // top-left
            { 0.0f,  0.0f, 0.0f, 0.0f, 0.0f }, // bottom-left
            { 1.0f,  1.0f, 0.0f, 1.0f, 1.0f }, // top-right
            { 1.0f,  0.0f, 0.0f, 1.0f, 0.0f }  // bottom-right
		};
		quad_update = std::make_unique<MeshData>(GL_TRIANGLE_STRIP, 4, 0, attr);
		quad_update->set_vertices(reinterpret_cast<GLfloat*>(verts));

        // build initial height and normal maps 
        auto max_index = update_elevation_maps();
        update_normal_maps(max_index);
    }

	void ClipMap::build_levels(void)
	{
		const auto min_z = 0.0f;
		const auto max_z = height_map->get_scale_factor();
		const auto inner_size = (level_size + 1) / 2;
		const auto block_size = (level_size + 1) / 4;

		for (int i = 0; i < num_levels; i++)
		{
			ClipMapLevel level(i);

			if (i == 0)
			{
				level.orientation = ClipMapLevel::bottom_right;
				level.scale = 1.0f;
				level.origin = Vector2{ 0.0f, 0.0f };
			}
			else
			{
				auto& prev_level = levels[i - 1];

				// Origin of each level is shifted by size of next finer level 
				// in both x and z direction (sign based on right / bottom flags)
				level.orientation = (prev_level.orientation == ClipMapLevel::bottom_right)
					? ClipMapLevel::top_left : ClipMapLevel::bottom_right;
				level.scale = 2.0f * prev_level.scale;

				// Check if orientation is left
				auto x_shift = (prev_level.orientation & 0x2) == 0x0
					? -level.scale * (float)(block_size - 1) : -level.scale * block_size;
				// Check if oriengation is up
				auto y_shift = (prev_level.orientation & 0x1) == 0x0
					? -level.scale * (float)(block_size - 1) : -level.scale * block_size;
				level.origin = Vector2{ prev_level.origin.x + x_shift, prev_level.origin.y + y_shift };
			}
			level.width = level.scale * (float)(level_size - 1);
			level.half_width = 0.5f * level.width;
				
			// generate inner boxes 
			for (int i = ClipMapLevel::bb_inner_idx; i < ClipMapLevel::bb_block_idx; i++)
			{
				auto tmp = level.origin + inner_offsets[i - ClipMapLevel::bb_inner_idx] * level.scale;
				level.bounding_boxes[i].min = Vector3{ tmp.x, min_z, tmp.y };
				level.bounding_boxes[i].max = Vector3{ tmp.x + (inner_size - 1) * level.scale, max_z, tmp.y + (inner_size - 1) * level.scale };
			}

			// generates block boxes 
			for (int i = ClipMapLevel::bb_block_idx; i < 16; i++)
			{
				auto tmp = level.origin + block_offsets[i - ClipMapLevel::bb_block_idx] * level.scale;
				level.bounding_boxes[i].min = Vector3{ tmp.x, min_z, tmp.y };
				level.bounding_boxes[i].max = Vector3{ tmp.x + (block_size - 1) * level.scale, max_z, tmp.y + (block_size - 1) * level.scale };
			}

			levels.push_back(level);
		}
	}

    void ClipMap::build_buffers(void)
    {
		// Meshes forming the clipmap geometry:
		// 
		//  B B R R B B 
		//  B F F F F B
		//  R I I I F R
		//  R I I I F R
		//  B I I I F B
		//  B B R R B B 
		// 

        const auto inner_size = (level_size + 1) / 2;
        const auto inner_width = (float)(inner_size - 1);
        
        // compute offsets for 4 inner buffers in row-order from top-left to bottom-right
        inner_offsets[0] = { inner_width, inner_width };
        inner_offsets[1] = { 0.0f, inner_width };
        inner_offsets[2] = { inner_width, 0.0f };
        inner_offsets[3] = { 0.0f, 0.0f };

        // build innermost buffer
        BlockBuilder bb;
        bb.add_block(inner_size, inner_size);
        inner_mesh = bb.create_mesh();

        // compute offsets of all 12 blocks in row-order from top-left to bottom-right
        const auto block_size = (level_size + 1) / 4;
        const auto block_width = (float)(block_size - 1);
        block_offsets[0] = { 2.0f + 3.0f * block_width, 2.0f + 3.0f * block_width };
        block_offsets[1] = { 2.0f + 2.0f * block_width, 2.0f + 3.0f * block_width };
        block_offsets[2] = { block_width, 2.0f + 3.0f * block_width };
        block_offsets[3] = { 0.0f, 2.0f + 3.0f * block_width };
        block_offsets[4] = { 2.0f + 3.0f * block_width, 2.0f + 2.0f * block_width };
        block_offsets[5] = { 0.0f, 2.0f + 2.0f * block_width };
        block_offsets[6] = { 2.0f + 3.0f * block_width, block_width };
        block_offsets[7] = { 0.0f, block_width };
        block_offsets[8] = { 2.0f + 3.0f * block_width, 0.0f };
        block_offsets[9] = { 2.0f + 2.0f * block_width, 0.0f };
        block_offsets[10] = { block_width, 0.0f };
        block_offsets[11] = { 0.0f, 0.0f };

        // build block buffer
        bb.clear();
        bb.add_block(block_size, block_size);
        block_mesh = bb.create_mesh();        

        // Create remaining fill primitive buffers.
        build_ring_buffer(block_size);
        build_fill_buffer(block_size);
        build_perimeter_buffer();
    }

    void ClipMap::build_ring_buffer(int block_size)
    {
        BlockBuilder bb;
        // TOP
        bb.add_block(3, block_size, 2 * (block_size - 1), 2 + 3 * (block_size - 1));
        // RIGHT
        bb.add_block(block_size, 3, 0, 2 * (block_size - 1));
        // BOTTOM
        bb.add_block(3, block_size, 2 * (block_size - 1), 0);
        // LEFT
        bb.add_block(block_size, 3, 2 + 3 * (block_size - 1), 2 * (block_size - 1));
        ring_mesh = bb.create_mesh();
    }

    void ClipMap::build_fill_buffer(int block_size)
    {
        // TODO: all this data could share 1 index buffer if we were to just rotate the
        // vertices around
        BlockBuilder bb;
        // Fill for BOTTOM-RIGHT
        bb.add_block(2, 2 * block_size, 1 + 3 * (block_size - 1), block_size - 1);
        bb.add_block(2 * block_size + 1, 2, block_size - 1, 1 + 3 * (block_size - 1));
        fill_mesh[0] = bb.create_mesh();
        //  Fill for TOP-RIGHT
        bb.clear();
        bb.add_block(2 * block_size + 1, 2, block_size - 1, block_size - 1);
        bb.add_block(2, 2 * block_size, 1 + 3 * (block_size - 1), block_size);
        fill_mesh[1] = bb.create_mesh();
        //  Fill for BOTTOM-LEFT
        bb.clear();
        bb.add_block(2, 2 * block_size, block_size - 1, block_size - 1);
        bb.add_block(2 * block_size + 1, 2, block_size - 1, 1 + 3 * (block_size - 1));
        fill_mesh[2] = bb.create_mesh();
        // Fill for TOP-LEFT
        bb.clear();
        bb.add_block(2 * block_size + 1, 2, block_size - 1, block_size - 1);
        bb.add_block(2, 2 * block_size, block_size - 1, block_size);
        fill_mesh[3] = bb.create_mesh();
    }

    void ClipMap::build_perimeter_buffer(void)
    {
        std::vector<GLshort> vertex_data;
        std::vector<GLushort> index_data;

        // BOTTOM
        for (int i = 0; i < level_size; i++)
        {
            vertex_data.push_back(static_cast<GLshort>(i));
            vertex_data.push_back(static_cast<GLshort>(0));
        }
        // LEFT
        for (int i = 1; i < level_size; i++)
        {
            vertex_data.push_back(static_cast<GLshort>(level_size - 1));
            vertex_data.push_back(static_cast<GLshort>(i));
        }
        // TOP
        for (int i = level_size - 2; i >= 0; i--)
        {
            vertex_data.push_back(static_cast<GLshort>(i));
            vertex_data.push_back(static_cast<GLshort>(level_size - 1));
        }
        // RIGHT
        for (int i = level_size - 2; i > 0; i--)
        {
            vertex_data.push_back(static_cast<GLshort>(0));
            vertex_data.push_back(static_cast<GLshort>(i));
        }

        auto vc = vertex_data.size() / 2;
        for (auto i = 0u; i < vc - 2; i += 2)
        {
            index_data.push_back(i);
            index_data.push_back(i + 1);
            index_data.push_back(i + 2);
        }

        // final triangle uses first vertex again
        index_data.push_back(static_cast<GLushort>(vc - 2));
        index_data.push_back(static_cast<GLushort>(vc - 1));
        index_data.push_back(0);

        std::vector<VertexAttribute> attributes;
		attributes.push_back(VertexAttribute(Renderer::at_pos, 2, 0, GL_SHORT));
        perimeter_mesh = std::make_unique<MeshData>(GL_TRIANGLES, vc, static_cast<int>(index_data.size()), attributes);
        perimeter_mesh->set_vertices(vertex_data);
        perimeter_mesh->set_indices(index_data);
    }

    void ClipMap::update(float delta)
    {
        // determine height of observer and set min_level accordingly
		auto height = height_map->get_scale_factor() * height_map->get_elevation((int)std::round(observer_pos.x), (int)std::round(observer_pos.z), 0);
		min_level = 0;
		while (min_level < (int)levels.size() && levels[min_level].width < 2.5f * (observer_pos.y - height))
		{
			min_level++;
		}

        // Update level origins, then samplers, then normal maps
        update_levels();
        auto max_index = update_elevation_maps();
        update_normal_maps(max_index);
    }

    void ClipMap::update_levels(void)
    {
        // Check if we meed to update the origin of the most fine-grained level.
        const uint8_t down = 1;
        const uint8_t up = 2;
        const uint8_t right = 4;
        const uint8_t left = 8;
        uint8_t update_flags = 0;

        auto it = levels.begin();

        // Threshold for update is: 2 grid spaces * level.scale 
        auto grid_size = 2.0f * (*it).scale;
        // compute delta between origin and camera 
        auto dx = observer_pos.x - ((*it).origin.x + (*it).half_width);
        auto dy = observer_pos.z - ((*it).origin.y + (*it).half_width);
        // Moving left
        if (dx > grid_size)
            update_flags |= left;
        // Moving right
        else if (dx < -grid_size)
            update_flags |= right;
        // Moving up
        if (dy > grid_size)
            update_flags |= up;
        // Moving down
        else if (dy < -grid_size)
            update_flags |= down;

        Vector2 offset_shift;
        while (it != levels.end() && update_flags != 0)
        {
            auto& level = *it;
            level.is_dirty = true;
            // Each test will move the origin of the current level and then
            // check the level's orientation within the next coarser level
            // to see if that one needs to be updated as well.
            if ((update_flags & left) == left)
            {
                offset_shift.x = grid_size;
                if (level.is_right())
                {
                    level.orientation |= 0x2; // flip to left side
                    update_flags &= (~left); // clear left update flag
                }
                else
                {
                    level.orientation &= 0x1;
                }
            }
            else if ((update_flags & right) == right) 
            {
                offset_shift.x = -grid_size;
                if (level.is_left())
                {
                    level.orientation &= 0x1; // flip to right side 
                    update_flags &= (~right);
                }
                else
                {
                    level.orientation |= 0x2;
                }
            }
            else
            {
                offset_shift.x = 0.0f;
            }

            if ((update_flags & up) == up)
            {
                offset_shift.y = grid_size;
                if (level.is_bottom())
                {
                    level.orientation |= 0x1; // flip to top side
                    update_flags &= (~up);
                }
                else
                {
                    level.orientation &= 0x2;
                }
            }
            else if ((update_flags & down) == down)
            {
                offset_shift.y = -grid_size;
                if (level.is_top())
                {
                    level.orientation &= 0x2; // flip to bottom side
                    update_flags &= (~down);
                }
                else
                {
                    level.orientation |= 0x1;
                }
            }
            else
            {
                offset_shift.y = 0.0f;
            }

            level.translate(offset_shift);

            grid_size *= 2.0f;
            ++it;
        }
    }

    int ClipMap::update_elevation_maps(void)
    {
        bool fbo_bound = false;
        int max_index = -1;

        // This will update all elevation samplers which are flagged as dirty. If the 
        // last_shift for a level is 0, the full texture if updated. Otherwise, this
        // will refresh only the updated section using torroidal addressing. Depending
        // on the last_shift motion (x, y, or x+y) we will render 1-4 quads to update 
        // the elevation sampler with new data.

        for (auto& level : levels)
        {
            if (!level.is_dirty)
                continue;

            // only bind FBO once
            if (!fbo_bound)
            {
                fb_update->bind();

                // Switch shader and bind uniforms
			    game->get_renderer()->switch_shader(update_program);
                auto one_over_size = 1.0f / (float)texture_size;
                glUniform2f(update_program->attr(uniform_size), (float)texture_size, (float)texture_size);
                glUniform2f(update_program->attr(uniform_one_over_size), one_over_size, one_over_size);

                // Bind update texture
                update_texture->bind(0, update_program);

                fbo_bound = true;       
            }

            // If this level hasn't been translated, perform a full rebuild of level
            if (level.last_shift.x == 0.0f && level.last_shift.y == 0.0f)
            {
                Rect r = { 
                    // need to convert origin from world to texture coordinates
                    (int)std::floor(level.origin.x / level.scale), 
                    (int)std::floor(level.origin.y / level.scale), 
                    level_size + 1, level_size + 1
                };

                //logger << "Rebuild " << level.index << " [" << r.x << "," << r.y << "]" << std::endl;
                height_map->get_data(level.index, r, buffer);
                elevation_maps->bind(0);
                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, level.index, r.w, r.h, 1,
                    GL_RED, GL_FLOAT, buffer.data());

                level.u = 0; level.v = 0;
            }
            else
            {
                // Sampling rect in world space
                Rect r_world;
                // Rect to replace in texture space
                Rect r_texture;

				// Make layer elevation map render target for framebuffer
                glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, elevation_maps->id, 0, level.index);

                // Compute new texture offset
                auto last_u = level.u;
                auto last_v = level.v;
                level.u = pos_mod(level.u + (int)level.last_shift.x, texture_size);
                level.v = pos_mod(level.v + (int)level.last_shift.y, texture_size);

                // horizontal shift
                if (level.last_shift.x != 0.0f)
                {
                    if (level.last_shift.x < 0.0f) 
                    {
                        r_world.x = (int)std::floor(level.origin.x / level.scale);
                        r_world.y = (int)std::floor(level.origin.y / level.scale);
                        r_world.w = (int)std::abs(level.last_shift.x);
                        r_world.h = texture_size;
                        r_texture.x = level.u;
                        r_texture.y = level.v;
                        r_texture.w = r_world.w;
                        r_texture.h = texture_size;
                    }
                    else 
                    {
                        r_world.x = (int)(std::floor(level.origin.x / level.scale) - level.last_shift.x) + texture_size;
                        r_world.y = (int)std::floor(level.origin.y / level.scale);
                        r_world.w = (int)std::abs(level.last_shift.x);
                        r_world.h = texture_size;
                        r_texture.x = last_u;
                        r_texture.y = level.v;
                        r_texture.w = r_world.w;
                        r_texture.h = texture_size;
                    }

                    // Fill texture with height data
                    height_map->get_data(level.index, r_world, buffer);
                    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, r_world.w, r_world.h, GL_RED, GL_FLOAT, buffer.data());

                    // Render quad to fill in new area
                    glUniform4f(update_program->attr(uniform_rect), 
                        (float)r_texture.x, (float)r_texture.y, (float)r_texture.w, (float)r_texture.h);
                    quad_update->render(update_program);

                    if (r_texture.y != texture_size)
                    {
                        glUniform4f(update_program->attr(uniform_rect), 
                            (float)r_texture.x, (float)(r_texture.y - texture_size), (float)r_texture.w, (float)r_texture.h);
                        quad_update->render(update_program);
                    }
                }

                // vertical shift
                if (level.last_shift.y != 0.0f)
                {
                    if (level.last_shift.y < 0.0f) 
                    {
                        r_world.x = (int)std::floor(level.origin.x / level.scale);
                        r_world.y = (int)std::floor(level.origin.y / level.scale);
                        r_world.w = texture_size;
                        r_world.h = (int)std::abs(level.last_shift.y);
                        r_texture.x = level.u;
                        r_texture.y = level.v;
                        r_texture.w = texture_size;
                        r_texture.h = r_world.h;
                    }
                    else 
                    {
                        r_world.x = (int)std::floor(level.origin.x / level.scale);
                        r_world.y = (int)(std::floor(level.origin.y / level.scale) - level.last_shift.y) + texture_size;
                        r_world.w = texture_size;
                        r_world.h = (int)std::abs(level.last_shift.y);
                        r_texture.x = level.u;
                        r_texture.y = last_v;
                        r_texture.w = texture_size;
                        r_texture.h = r_world.h;
                    }

                    // Fill texture with height data
                    height_map->get_data(level.index, r_world, buffer);
                    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, r_world.w, r_world.h, GL_RED, GL_FLOAT, buffer.data());

                    // Render quad to fill in new area
                    glUniform4f(update_program->attr(uniform_rect), 
                        (float)r_texture.x, (float)r_texture.y, (float)r_texture.w, (float)r_texture.h);
                    quad_update->render(update_program);

                    if (r_texture.x != texture_size)
                    {
                        glUniform4f(update_program->attr(uniform_rect), 
                            (float)(r_texture.x - texture_size), (float)r_texture.y, (float)r_texture.w, (float)r_texture.h);
                        quad_update->render(update_program);
                    }
                }

                perfc.inc(PerformanceCounter::FRAME_BUFFERS);
            }

            level.is_dirty = false;
            max_index = level.index;
        }

        if (fbo_bound)
        {
            fb_update->unbind();
        }

        return max_index;
    }

    void ClipMap::update_normal_maps(int max_index)
    {
        if (max_index < 0)
            return;

        // Regenerate normal maps from coarsest to finest
        fb_normal->bind();

        // Switch shader and bind uniforms
        game->get_renderer()->switch_shader(normal_program);

        // Inputs are elevation maps (GL_TEXTURE0) and normal maps (GL_TEXTURE1)
        elevation_maps->bind(0, normal_program);
        // TODO: review - doesn't appear to be used in the shader
        normal_maps->bind(1, normal_program);

        // size of normal map
        glUniform1f(normal_program->attr(uniform_size), (float)(2 * texture_size));
        // 1 / normal map texture size 
        const auto one_over_size = 1.0f / (float)(2 * texture_size);
        glUniform1f(normal_program->attr(uniform_one_over_size), one_over_size);
        // pass in ratio of z to x/y grid spacing
        glUniform2f(normal_program->attr("u_grid_scale"),
            -0.5f * height_map->get_scale_factor(),
            -0.5f * height_map->get_scale_factor());

        for (int i = max_index; i >= 0; i--)
        {
            // Level specific uniforms
            glUniform1i(normal_program->attr("u_level"), i);
			// Pass in torroidial texture offset to handle artifacts along
			// the border of the normal map. Multiply by 2 since normal map
			// is twice the size of elevation sampler.
			glUniform2f(normal_program->attr(uniform_tex_offset), 
				2.0f * (float)levels[i].u, 2.0f * (float)levels[i].v);
            
            // Make layer normal map render target for framebuffer
            glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, normal_maps->id, 0, i);
            
            quad_normal->render(normal_program);

            perfc.inc(PerformanceCounter::FRAME_BUFFERS);
        }

        fb_normal->unbind();
    }

    void ClipMap::render_level(const Camera3& cam, int level_idx)
    {
        const auto& level = levels[level_idx];

        // Transition width - determines how wide the area of blending is
        auto w = level.width / 10.0f;
        // Pre-computed blend parameters - used to blend geometry with coarser level 
        // at outer border of finer level.
        auto alpha_offset = level.half_width - w - level.scale * 2.0f;
        auto one_over_w = 1.0f / w;

        // We're repurposing the model matrix to pass in information about the
        // location and scale of each rendered block.
        Matrix4 model;
        // grid scale at current level
        model.m[0] = model.m[1] = level.scale;
        // origin of current block in world-space (set below for each block)
        model.m[2] = model.m[3] = 0.0f;
        // 1 / texture width,height
        model.m[4] = model.m[5] = 1.0f / (float)texture_size;
        // origin of current block in texture space (set below for each block)
        model.m[6] = model.m[7] = 0.0f;
        // Observer position 
        model.m[8] = observer_pos.x; model.m[9] = observer_pos.z;
        // Alpha offset 
        model.m[10] = model.m[11] = alpha_offset;
        // 1/w
        model.m[12] = one_over_w; 
        // ZScale of height map 
        model.m[13] = height_map->get_scale_factor(); 
        // level index and max levels
        model.m[14] = (float)level.index; model.m[15] = (float)num_levels - 1.0f;

		// Debug parameters
		glUniform2f(program->attr("u_debug"), 
			blending ? 1.0f : 0.0f,
			lighting ? 1.0f : 0.0f);
        
		// debug color
		auto color_factor = 1.0f - (float)level.index / (float)num_levels;
		glUniform4f(program->attr("u_color"), 0.0f, 0.0f, color_factor, 1.0f);

		// Pass in texture offset of current level within coarser level
        auto base_offset_x = (level_idx + 1) < num_levels ? (float)levels[level_idx + 1].u : 0.0f;
        auto base_offset_y = (level_idx + 1) < num_levels ? (float)levels[level_idx + 1].v : 0.0f;
		glUniform4f(program->attr(uniform_tex_offset),
            (float)level.u * model.m[4], (float)level.v * model.m[5],
			base_offset_x * model.m[4], base_offset_y * model.m[5]);
		// We need to adjust for orientation of this level within the coarser level
		// Offset is either width of 1 block of parent or width of 1 block + 1
		auto block_size = texture_size / 4;
		glUniform2f(program->attr("u_offset"),
			(level.is_right() ? (float)block_size - 1.0f : (float)block_size) * model.m[4],
			(level.is_bottom() ? (float)block_size - 1.0f : (float)block_size) * model.m[5]);

        // Handle min level as special case
        if (level_idx == min_level)
        {
            // Render innermost level
            for (int i = 0; i < 4; i++)
            {
                if (culling && cam.is_clipped(level.bounding_boxes[ClipMapLevel::bb_inner_idx + i]))
                {
                    continue;
                }            
                model.m[2] = level.origin.x + (inner_offsets[i].x * level.scale); 
                model.m[3] = level.origin.y + (inner_offsets[i].y * level.scale); 
                model.m[6] = inner_offsets[i].x * model.m[4]; 
                model.m[7] = inner_offsets[i].y * model.m[5];
                glUniformMatrix4fv(program->attr(Renderer::uf_model), 1, false, model.m);
                inner_mesh->render(program);
            }
        }
        else
        {
            // Render 12 <B> blocks
            for (int i = 0; i < 12; i++)
            {
                if (culling && cam.is_clipped(level.bounding_boxes[ClipMapLevel::bb_block_idx + i]))
                {
                    continue;
                }            
                model.m[2] = level.origin.x + (block_offsets[i].x * level.scale); 
                model.m[3] = level.origin.y + (block_offsets[i].y * level.scale); 
                model.m[6] = block_offsets[i].x * model.m[4]; 
                model.m[7] = block_offsets[i].y * model.m[5];
                glUniformMatrix4fv(program->attr(Renderer::uf_model), 1, false, model.m);
                block_mesh->render(program);
            }

            // Render 4 <F> blocks
            glUniform4f(program->attr("u_color"), 0.0f, color_factor, 0.0f, 1.0f);
            model.m[2] = level.origin.x; model.m[3] = level.origin.y; 
            model.m[6] = 0.0f; model.m[7] = 0.0f;
            glUniformMatrix4fv(program->attr(Renderer::uf_model), 1, false, model.m);
            ring_mesh->render(program);
        
            // Render interior block based on orientation of next finer level
            glUniform4f(program->attr("u_color"), color_factor, 0.0f, 0.0f, 1.0f);
            model.m[2] = level.origin.x; model.m[3] = level.origin.y; 
            model.m[6] = 0.0f; model.m[7] = 0.0f;
            glUniformMatrix4fv(program->attr(Renderer::uf_model), 1, false, model.m);
            auto buffer_idx = (int)levels[level_idx - 1].orientation;
            fill_mesh[buffer_idx]->render(program);
        }

        if (stitching)
        {
			glUniform4f(program->attr("u_color"), 1.0f, 1.0f, 1.0f, 1.0f);
			model.m[2] = level.origin.x; model.m[3] = level.origin.y;
            model.m[6] = 0.0f; model.m[7] = 0.0f;
            glUniformMatrix4fv(program->attr(Renderer::uf_model), 1, false, model.m);
            perimeter_mesh->render(program);
        }
    }

    void ClipMap::render(Renderer* renderer) 
    {
		auto cam = dynamic_cast<Renderer3*>(renderer)->get_camera();

        renderer->switch_shader(program);

        // Bind samplers
        elevation_maps->bind(0, program);
        normal_maps->bind(1, program);
        color_map->bind(2, program);

        // Render primitives for each visible level 
        for (auto i = min_level; i < num_levels; i++)
        {
            render_level(*cam, i);
        }

        perfc.inc(PerformanceCounter::MESHES);

        color_map->unbind();
        normal_maps->unbind();
        elevation_maps->unbind();
    }

	void ClipMap::set_palette(const std::vector<Color>& palette)
	{
		color_map = std::make_unique<Texture>(palette.size(), 1);
		color_map->target = GL_TEXTURE_1D;
		color_map->bind(0);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, color_map->w, 0, GL_RGBA, GL_FLOAT, palette.data());
	}
}