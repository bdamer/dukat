#include "stdafx.h"

#include "blockbuilder.h"
#include "game3.h"
#include "log.h"
#include "mathutil.h"
#include "meshbuilder2.h"
#include "meshinstance.h"
#include "textureutil.h"
#include "renderer3.h"
#include "vertextypes3.h"
#include "wavemesh.h"

namespace dukat
{
	constexpr int WaveMesh::texture_size;
	constexpr float WaveMesh::grav_constant;

	WaveMesh::WaveMesh(Game3* game, int size) : game(game), grid_size(size), tile_spacing(1.0f), 
		env_map(nullptr), elev_map(nullptr)
	{
		init_cos_table();
		noise_texture = generate_noise_texture(texture_size, texture_size);
		reset_state();
		init_waves();
		
		// Set up mesh, framebuffer, and texture for wave texture map
		MeshBuilder2 mb;
		fb_quad = mb.build_textured_quad();
		fbo = std::make_unique<FrameBuffer>(texture_size, texture_size, false, false);
		fb_texture = std::make_unique<Texture>(texture_size, texture_size, ProfileLinear);
		glBindTexture(GL_TEXTURE_2D, fb_texture->id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, texture_size, texture_size, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		fb_program = game->get_shaders()->get_program("fx_water.vsh", "fx_water.fsh");

		// Set up mesh and texture for geo texture
		BlockBuilder bb;
		bb.add_block(grid_size, grid_size);
		grid_mesh = bb.create_mesh();
		grid_program = game->get_shaders()->get_program("sc_waves.vsh", "sc_waves.fsh");
	}

	void WaveMesh::init_cos_table(void)
	{
		std::vector<uint32_t> cos_table(texture_size);
		for (int i = 0; i < texture_size; i++)
		{
			auto dist = (float)i / (float)(texture_size - 1) * two_pi;
			auto c = std::cos(dist);
			auto s = std::cos(dist);
			s *= 0.5f;
			s += 0.5f;
			s = std::pow(s, tex_state.chop);
			c *= s;
			uint8_t cos_dist = static_cast<uint8_t>((c * 0.5f + 0.5f) * 255.999f);
			cos_table[i] = (cos_dist << 24) | (cos_dist << 16) | (0xff << 8) | 0xff;
		}
		// Upload to texture
		cos_texture = std::make_unique<Texture>(texture_size, 1, ProfileNearest);
		cos_texture->target = GL_TEXTURE_1D;
		glBindTexture(cos_texture->target, cos_texture->id);
		glTexParameteri(cos_texture->target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(cos_texture->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage1D(cos_texture->target, 0, GL_RGBA8, texture_size, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, cos_table.data());
	}

	void WaveMesh::reset_state(void)
	{
		// Init Geo wave state
		geo_state.chop = 2.5f;
		geo_state.angle_deviation = 15.f;
		geo_state.wind_dir = Vector2{ 0.0f, 1.0f };
		geo_state.min_length = 15.f;
		geo_state.max_length = 25.f;
		geo_state.amp_over_len = 0.1f;
		geo_state.env_radius = 200.f;
		geo_state.water_level = 2.0f;
		geo_state.trans_idx = 0;
		geo_state.trans_del = -1.f / 6.f;
		geo_state.spec_atten = 1.f;
		geo_state.spec_end = 200.f;

		geo_state.spec_trans = 100.f;

		// Init Tex wave state
		tex_state.noise = 0.2f;
		tex_state.chop = 1.f;
		tex_state.angle_deviation = 15.f;
		tex_state.wind_dir = Vector2{ 0.0f, 1.0f };
		tex_state.max_length = 10.f;
		tex_state.min_length = 1.f;
		tex_state.amp_over_len = 0.1f;
		tex_state.ripple_scale = 64.0f;
		tex_state.speed_deviation = 0.1f;
		tex_state.trans_idx = 0;
		tex_state.trans_del = -1.f / 5.f;
	}

	void WaveMesh::init_waves(void)
	{
		for (int i = 0; i < num_geo_waves; i++)
			init_geo_wave(i);

		for (int i = 0; i < num_tex_waves; i++)
			init_tex_wave(i);
	}

	void WaveMesh::init_geo_wave(int i)
	{
		geo_waves[i].phase = randf(0.0f, two_pi);
		geo_waves[i].len = randf(geo_state.min_length, geo_state.max_length);
		geo_waves[i].amp = geo_waves[i].len * geo_state.amp_over_len / static_cast<float>(num_geo_waves);
		geo_waves[i].freq = two_pi / geo_waves[i].len;
		geo_waves[i].fade = 1.f;

		auto rot_base = geo_state.angle_deviation * pi / 180.f;
		auto rads = rot_base * randf(-1.0f, 1.0f);
		auto rx = std::cos(rads);
		auto ry = std::sin(rads);
		auto x = -geo_state.wind_dir.x;
		auto y = -geo_state.wind_dir.y;
		geo_waves[i].dirx = x * rx + y * ry;
		geo_waves[i].diry = x * -ry + y * rx;
		geo_waves[i].dirz = 0.0f;
	}

	void WaveMesh::init_tex_wave(int i)
	{
		auto rads = randf(-1.0f, 1.0f) * tex_state.angle_deviation * pi / 180.f;
		auto rx = std::cos(rads);
		auto ry = std::sin(rads);
		auto dx = tex_state.wind_dir.x * rx + tex_state.wind_dir.y * ry;
		auto dy = tex_state.wind_dir.x * -ry + tex_state.wind_dir.y * rx;

		auto max_len = tex_state.max_length * texture_size / tex_state.ripple_scale;
		auto min_len = tex_state.min_length * texture_size / tex_state.ripple_scale;
		auto len = static_cast<float>(i) / static_cast<float>(num_tex_waves - 1) * (max_len - min_len) + min_len;

		auto reps = texture_size / len;
		dx *= reps;
		dy *= reps;
		dx = static_cast<float>(int(dx >= 0 ? dx + 0.5f : dx - 0.5f));
		dy = static_cast<float>(int(dy >= 0 ? dy + 0.5f : dy - 0.5f));

		tex_waves[i].rot_scale.x = dx;
		tex_waves[i].rot_scale.y = dy;

		auto eff_k = 1.0f / std::sqrt(dx*dx + dy*dy);
		tex_waves[i].len = static_cast<float>(texture_size) * eff_k;
		tex_waves[i].freq = two_pi / tex_waves[i].len;
		tex_waves[i].amp = tex_waves[i].len * tex_state.amp_over_len;
		tex_waves[i].phase = randf(0.0f, 1.0f);

		tex_waves[i].dir.x = dx * eff_k;
		tex_waves[i].dir.y = dy * eff_k;

		tex_waves[i].fade = 1.0f;

		auto speed = (1.0f / std::sqrt(tex_waves[i].len / (two_pi * grav_constant))) / 3.0f;
		speed *= 1.0f + randf(-tex_state.speed_deviation, tex_state.speed_deviation);
		tex_waves[i].speed = speed;
	}

	void WaveMesh::update(float delta)
	{
		// Update Tex Wave
		for (auto i = 0; i < num_tex_waves; i++)
		{
			if (i == tex_state.trans_idx)
			{
				tex_waves[i].fade += tex_state.trans_del * delta;
				if (tex_waves[i].fade < 0.0f)
				{
					// This wave is faded out. Re-init and fade it back up.
					init_tex_wave(i);
					tex_waves[i].fade = 0.0f;
					tex_state.trans_del = -tex_state.trans_del;
				}
				else if (tex_waves[i].fade > 1.0f)
				{
					// This wave is faded back up. Start fading another down.
					tex_waves[i].fade = 1.f;
					tex_state.trans_del = -tex_state.trans_del;
					if (++tex_state.trans_idx >= num_tex_waves)
						tex_state.trans_idx = 0;
				}
			}
			tex_waves[i].phase -= delta * tex_waves[i].speed;
			tex_waves[i].phase -= (int)tex_waves[i].phase;
		}

		// Update Geo Wave
		for (auto i = 0; i < num_geo_waves; i++)
		{
			if (i == geo_state.trans_idx)
			{
				geo_waves[i].fade += geo_state.trans_del * delta;
				if (geo_waves[i].fade < 0)
				{
					// This wave is faded out. Re-init and fade it back up.
					init_geo_wave(i);
					geo_waves[i].fade = 0;
					geo_state.trans_del = -geo_state.trans_del;
				}
				else if (geo_waves[i].fade > 1.0f)
				{
					// This wave is faded back up. Start fading another down.
					geo_waves[i].fade = 1.f;
					geo_state.trans_del = -geo_state.trans_del;
					if (++geo_state.trans_idx >= num_geo_waves)
						geo_state.trans_idx = 0;
				}
			}

			const auto speed = 1.0f / std::sqrt(geo_waves[i].len / (two_pi * grav_constant));
			geo_waves[i].phase += speed * delta;
			geo_waves[i].phase = std::fmod(geo_waves[i].phase, two_pi);
			geo_waves[i].amp = geo_waves[i].len * geo_state.amp_over_len / (float)num_geo_waves * geo_waves[i].fade;
		}
	}

    void WaveMesh::update_framebuffer(Renderer* renderer)
	{
        glDisable(GL_BLEND);

        fbo->bind();
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb_texture->id, 0);
		
        renderer->switch_shader(fb_program);
		
		// Used to scale normals to output format
		auto s = 0.5f / (static_cast<float>(num_bump_per_pass) + tex_state.noise);
		fb_program->set("u_rescale", s, s, 1.0f, 1.0f);
		
		cos_texture->bind(0, fb_program); // consine lookup
		noise_texture->bind(1, fb_program); // noise texture
		fb_texture->bind(2, fb_program); // output buffer
		
		// wave passes
		Matrix4 mat_model;
		for (int i = 0; i < 4; i++) 
		{
			fb_program->set("u_pass", static_cast<float>(i));

			// Set cUTransX
			mat_model.m[0] = tex_waves[i * 4 + 0].rot_scale.x;
			mat_model.m[1] = tex_waves[i * 4 + 0].rot_scale.y;
			mat_model.m[2] = 0.0f;
			mat_model.m[3] = tex_waves[i * 4 + 0].phase;
			mat_model.m[4] = tex_waves[i * 4 + 1].rot_scale.x;
			mat_model.m[5] = tex_waves[i * 4 + 1].rot_scale.y;
			mat_model.m[6] = 0.0f;
			mat_model.m[7] = tex_waves[i * 4 + 1].phase;
			mat_model.m[8] = tex_waves[i * 4 + 2].rot_scale.x;
			mat_model.m[9] = tex_waves[i * 4 + 2].rot_scale.y;
			mat_model.m[10] = 0.0f;
			mat_model.m[11] = tex_waves[i * 4 + 2].phase;
			mat_model.m[12] = tex_waves[i * 4 + 3].rot_scale.x;
			mat_model.m[13] = tex_waves[i * 4 + 3].rot_scale.y;
			mat_model.m[14] = 0.0f;
			mat_model.m[15] = tex_waves[i * 4 + 3].phase;
			fb_program->set_matrix4("u_rtex_coord", mat_model.m);

			// Set cCoefX
			auto norm_scale = tex_waves[i * 4 + 0].fade / (float)num_bump_passes;
			mat_model.m[0] = tex_waves[i * 4 + 0].dir.x * norm_scale;
			mat_model.m[1] = tex_waves[i * 4 + 0].dir.y * norm_scale;
			mat_model.m[2] = mat_model.m[3] = 1.0f;
			norm_scale = tex_waves[i * 4 + 1].fade / (float)num_bump_passes;
			mat_model.m[4] = tex_waves[i * 4 + 1].dir.x * norm_scale;
			mat_model.m[5] = tex_waves[i * 4 + 1].dir.y * norm_scale;
			mat_model.m[6] = mat_model.m[7] = 1.0f;
			norm_scale = tex_waves[i * 4 + 2].fade / (float)num_bump_passes;
			mat_model.m[8] = tex_waves[i * 4 + 2].dir.x * norm_scale;
			mat_model.m[9] = tex_waves[i * 4 + 2].dir.y * norm_scale;
			mat_model.m[10] = mat_model.m[11] = 1.0f;
			norm_scale = tex_waves[i * 4 + 3].fade / (float)num_bump_passes;
			mat_model.m[12] = tex_waves[i * 4 + 3].dir.x * norm_scale;
			mat_model.m[13] = tex_waves[i * 4 + 3].dir.y * norm_scale;
			mat_model.m[14] = mat_model.m[15] = 1.0f;
			fb_program->set_matrix4("u_coef", mat_model.m);
			
        	fb_quad->render(fb_program);
	        perfc.inc(PerformanceCounter::FRAME_BUFFERS);
		}
		// end wave passes

		// noise pass
		fb_program->set("u_pass", 4.0f);
		
		// Repurpose u_rescale to store scale bias
		auto scale_bias = 0.5f * tex_state.noise / (static_cast<float>(num_bump_passes) + tex_state.noise);
		fb_program->set("u_rescale", scale_bias, scale_bias, 0.0f, 1.0f);
		// Repurpose cUTransX to send uvXform parameters
		mat_model.m[0] = 20.0f; mat_model.m[4] = 0.0f; 		mat_model.m[8] = 20.0f; mat_model.m[12] = 0.0f;
		mat_model.m[1] = 0.0f; 	mat_model.m[5] = 20.0f; 	mat_model.m[9] = 0.0f; 	mat_model.m[13] = 20.0f;
		mat_model.m[2] = 		mat_model.m[6] = 			mat_model.m[10] = 		mat_model.m[14] = 0.0f;
		mat_model.m[3] = 		mat_model.m[7] = 			mat_model.m[11] = 		mat_model.m[15] = 0.1f * game->get_time();

		fb_program->set_matrix4("u_rtex_coord", mat_model.m);
		
		fb_quad->render(fb_program);
        perfc.inc(PerformanceCounter::FRAME_BUFFERS);
		// end noise pass

        fbo->unbind();
		
        glEnable(GL_BLEND);
	}

	void WaveMesh::render(Renderer* renderer)
	{
		update_framebuffer(renderer);

		renderer->switch_shader(grid_program);
		
		// determine intersection of camera eye ray and mesh
		auto cam = dynamic_cast<Renderer3*>(renderer)->get_camera();
		Vector3 cam_target;
		if (cam->transform.dir.y != 0.0f)
		{
			auto n = (geo_state.water_level - cam->transform.position.y) / cam->transform.dir.y;
			cam_target = cam->transform.position + n * cam->transform.dir;
		}
		else 
		{
			// eye ray is parallel to mesh plan, just use camera pos
			cam_target = cam->transform.position;
		}

		// use camera position to determine wave mesh offset
		auto x_offset = std::floor(cam_target.x / tile_spacing);
		auto z_offset = std::floor(cam_target.z / tile_spacing);

		// We're repurposing the model matrix to pass in information about the location and scale of the grid.
		// grid scale at current level
		Matrix4 mat_model;
		mat_model.m[0] = mat_model.m[1] = tile_spacing;
		// origin of mesh in world-space
		mat_model.m[2] = (-0.5f * static_cast<float>(grid_size) + x_offset) * tile_spacing;
		mat_model.m[3] = (-0.5f * static_cast<float>(grid_size) + z_offset) * tile_spacing;
		// 1 / texture width,height
		mat_model.m[4] = mat_model.m[5] = 1.0f / static_cast<float>(grid_size);
		// ZScale of height map 
		mat_model.m[13] = tile_spacing * scale_factor;
		grid_program->set_matrix4(Renderer::uf_model, mat_model);

		// Bind geo state
		grid_program->set("u_ws.water_tint", 0.05f, 0.1f, 0.1f, 0.5f);
		grid_program->set("u_ws.depth_offset",
			geo_state.water_level + 1.0f, geo_state.water_level + 1.0f, 
			geo_state.water_level, geo_state.water_level);
		grid_program->set("u_ws.fog_params", -200.0f, 1.0f / -100.0f, 0.0f, 1.0f);
		// Specular attenuation
		auto norm_scale = geo_state.spec_atten * tex_state.amp_over_len * two_pi;
		norm_scale *= (static_cast<float>(num_bump_passes) + tex_state.noise) * (tex_state.chop + 1.0f);
		grid_program->set("u_ws.spec_atten", geo_state.spec_end, 
			1.0f / geo_state.spec_trans, norm_scale, 1.0f / tex_state.ripple_scale);
		// Env adjust
		auto cam_to_center = cam->transform.position - cam_target;
		auto g = cam_to_center.mag2() - geo_state.env_radius * geo_state.env_radius;
		grid_program->set("u_ws.env_adjust", cam_to_center.x, cam_to_center.y, cam_to_center.z, g);

		auto k = 5.0f;
		if (geo_state.amp_over_len > geo_state.chop / (two_pi * num_geo_waves * k))
		{
			k = geo_state.chop / (two_pi * geo_state.amp_over_len * num_geo_waves);
		}
		grid_program->set("u_k", k);

		// Bind geo waves to uniform buffer
		renderer->bind_uniform(Renderer::UniformBuffer::USER, num_geo_waves * sizeof(GeoWaveDesc), geo_waves.data());
		grid_program->bind("Waves", Renderer::UniformBuffer::USER);

		// Set environment and bump map
		if (env_map != nullptr)
		{
			env_map->bind(0, grid_program);
		}
		if (elev_map != nullptr)
		{
			elev_map->bind(1, grid_program);
		}

		fb_texture->bind(2, grid_program);

		grid_mesh->render(grid_program);
	}

	// Samples wave mesh position; this code matches the code in the 
	// wave mesh vertex shader.
	float WaveMesh::sample(float x, float y) const
	{
		auto z = geo_state.water_level;
		auto scale = 0.5f * (geo_state.water_level);
		clamp(scale, 0.0f, 1.0f);

		// Sum effect of geo waves
		for (auto& wave : geo_waves) 
		{
			// Dot x and y with direction vectors
			auto dist = wave.dirx * x + wave.diry * y;
			// Scale in our frequency and add in our phase
			dist = dist * wave.freq + wave.phase;
			// Mod into range [-Pi..Pi]
			auto tmp = (dist + pi) * one_over_two_pi;
			dist = (tmp - std::floor(tmp)) * two_pi - pi;
			// Compute powers
			auto dist2 = dist * dist;
			auto dist3 = dist2 * dist;
			auto dist4 = dist2 * dist2;
			auto dist5 = dist3 * dist2;
			auto dist6 = dist3 * dist3;
			auto dist7 = dist4 * dist3;
			auto sine = dist - dist3 / 6.0f + dist5 / 120.0f - dist7 / 5040.0f;
			auto amp = wave.len / tile_spacing;
			clamp(amp, 0.0f, 1.0f);
			auto filtered_amp = amp * scale * wave.amp;
			z += sine * filtered_amp;
		}

		return z;
	}
}