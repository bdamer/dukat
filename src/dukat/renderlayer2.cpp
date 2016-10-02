#include "stdafx.h"
#include "renderlayer2.h"
#include "aabb2.h"
#include "buffers.h"
#include "camera2.h"
#include "effect2.h"
#include "matrix4.h"
#include "particle.h"
#include "perfcounter.h"
#include "shadercache.h"
#include "sprite.h"
#include "textmeshinstance.h"
#include "renderer2.h"
#include "vertextypes2.h"

namespace dukat
{
	// module-global buffer for particle data used during rendering
	static ParticleVertex2 particle_data[Renderer2::max_particles];

	RenderLayer2::RenderLayer2(ShaderCache* shader_cache, VertexBuffer* sprite_buffer, VertexBuffer* particle_buffer,
		const std::string& id, float priority, float parallax)
		: id(id), priority(priority), parallax(parallax), is_visible(true), 
		shader_cache(shader_cache), sprite_buffer(sprite_buffer), particle_buffer(particle_buffer)
	{
		sprite_program = shader_cache->get_program("sc_sprite.vsh", "sc_sprite.fsh");
		particle_program = shader_cache->get_program("sc_particle.vsh", "sc_particle.fsh");
	}

	RenderLayer2::~RenderLayer2(void)
	{
	}

	void RenderLayer2::add(Sprite* sprite)
	{
		sprites.push_back(sprite);
	}

	void RenderLayer2::remove(Sprite* sprite)
	{
		sprites.erase(std::remove(sprites.begin(), sprites.end(), sprite), sprites.end());
	}

	Effect2* RenderLayer2::add(std::unique_ptr<Effect2> fx)
	{
		auto res = fx.get();
		fx->set_layer(this);
		effects.push_back(std::move(fx));
		return res;
	}

	void RenderLayer2::remove(Effect2* fx)
	{
		fx->set_layer(nullptr);
		auto it = std::find_if(effects.begin(), effects.end(), 
			[fx](const std::unique_ptr<Effect2>& ptr) -> bool { return fx == ptr.get(); });
		if (it != effects.end())
		{
			effects.erase(it, effects.end());
		}
	}

	void RenderLayer2::add(Particle* particle)
	{
		particles.push_front(particle);
	}

	void RenderLayer2::remove(Particle* p)
	{
		particles.erase(std::remove(particles.begin(), particles.end(), p), particles.end());
	}

	void RenderLayer2::add(TextMeshInstance * text)
	{
		texts.push_back(text);
	}

	void RenderLayer2::remove(TextMeshInstance* text)
	{
		texts.erase(std::remove(texts.begin(), texts.end(), text), texts.end());
	}

	void RenderLayer2::render(Renderer2* renderer)
	{
		// Compute bounding box for current layer adjusted for parallax value
		auto camera = renderer->get_camera();
		Vector2 camera_pos = camera->transform.position * parallax;
		Vector2 camera_dim = camera->transform.dimension / 2.0f;
		AABB2 camera_bb(camera_pos - camera_dim, camera_pos + camera_dim);

		// TODO: set up per-layer lighting here

		if (has_effects())
		{
			render_effects(renderer, camera_bb);
		}
		if (has_sprites())
		{
			render_sprites(renderer, camera_bb);
		}
		if (has_particles())
		{
			render_particles(renderer, camera_bb);
		}
		if (has_text())
		{
			render_text(renderer, camera_bb);
		}
	}

	void RenderLayer2::fill_sprite_queue(const AABB2& camera_bb,
		std::priority_queue<Sprite*, std::deque<Sprite*>, SpriteComparator>& queue)
	{
		// Fill queue with sprites ordered by priority from low to high
		for (auto sprite : sprites)
		{
			Vector2 half_dim(sprite->scale * sprite->w / 2.0f, sprite->scale * sprite->h / 2.0f);
			AABB2 sprite_bb(sprite->p - half_dim, sprite->p + half_dim);
			if (!camera_bb.overlaps(sprite_bb))
			{
				sprite->rendered = false;
			}
			else
			{
				sprite->rendered = true;
				queue.push(sprite);
				perfc.inc(PerformanceCounter::SPRITES);
			}
		}
	}

	void RenderLayer2::render_sprites(Renderer2* renderer, const AABB2& camera_bb)
	{
		std::priority_queue<Sprite*, std::deque<Sprite*>, SpriteComparator> queue;
		fill_sprite_queue(camera_bb, queue);
		if (queue.empty())
		{
			return; // nothing to render
		}

		renderer->switch_shader(sprite_program);

		// Set parallax value for this layer
		glUniform1f(sprite_program->attr("u_parallax"), parallax);

		// bind sprite vertex buffers
		glBindVertexArray(sprite_buffer->vao);
		glBindBuffer(GL_ARRAY_BUFFER, sprite_buffer->buffers[0]);
		// bind vertex position
		auto pos_id = sprite_program->attr(Renderer::at_pos);
		glEnableVertexAttribArray(pos_id);
		glVertexAttribPointer(pos_id, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex2),
			reinterpret_cast<const GLvoid*>(offsetof(TexturedVertex2, x)));
		// bind texture position
		auto uv_id = sprite_program->attr(Renderer::at_texcoord);
		glEnableVertexAttribArray(uv_id);
		glVertexAttribPointer(uv_id, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex2),
			reinterpret_cast<const GLvoid*>(offsetof(TexturedVertex2, u)));
		// set texture unit 0 
		glUniform1i(sprite_program->attr(Renderer::uf_tex0), 0);
		
		// Get uniforms that will be set for each sprite
		auto uvwh_id = sprite_program->attr("u_uvwh");
		auto color_id = sprite_program->attr(Renderer::uf_color);
		auto model_id = sprite_program->attr(Renderer::uf_model);

		// Render in order
		GLuint last_texture = -1;
		while (!queue.empty())
		{
			// get top element from queue
			auto sprite = queue.top();
			queue.pop();

			// switch texture if necessary
			if (last_texture != sprite->texture_id)
			{
				perfc.inc(PerformanceCounter::TEXTURES);

				glActiveTextureARB(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, sprite->texture_id);
				last_texture = sprite->texture_id;
			}

			Matrix4 mat_m = sprite->compute_model_matrix();
			glUniformMatrix4fv(model_id, 1, false, &mat_m.m[0]);
			glUniform4fv(color_id, 1, &sprite->color.r);
			glUniform4fv(uvwh_id, 1, sprite->tex);
			
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

#ifdef _DEBUG
		// unbind buffers
		glDisableVertexAttribArray(pos_id);
		glDisableVertexAttribArray(uv_id);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
#endif
	}

	void RenderLayer2::render_particles(Renderer2* renderer, const AABB2& camera_bb)
	{
		int particle_count = 0;
		for (auto& it = particles.begin(); it != particles.end(); )
		{
			Particle* p = (*it);
			// remove dead particles
			if (p->ttl <= 0)
			{
				it = particles.erase(it);
				continue;
			}

			// check if particle visible and store result in ->rendered
			if (p->rendered = camera_bb.contains(p->pos))
			{
				particle_data[particle_count].x = p->pos.x;
				particle_data[particle_count].y = p->pos.y;
				particle_data[particle_count].size = p->size;
				particle_data[particle_count].r = p->color.r;
				particle_data[particle_count].g = p->color.g;
				particle_data[particle_count].b = p->color.b;
				particle_data[particle_count].a = p->color.a;
				perfc.inc(PerformanceCounter::PARTICLES);
				particle_count++;
			}
			++it;
		}

		if (particle_count == 0)
		{
			return; // no particles left to render
		}

		renderer->switch_shader(particle_program);

		// Set parallax value for this layer
		glUniform1f(particle_program->attr("u_parallax"), parallax);

		// bind particle vertex buffers
		glBindVertexArray(particle_buffer->vao);
		glBindBuffer(GL_ARRAY_BUFFER, particle_buffer->buffers[0]);
		// Orphan buffer to improve streaming performance
		glBufferData(GL_ARRAY_BUFFER, Renderer2::max_particles * sizeof(ParticleVertex2), NULL, GL_STREAM_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, particle_count * sizeof(ParticleVertex2), particle_data);
		// bind vertex position
		auto pos_id = particle_program->attr(Renderer::at_pos);
		glEnableVertexAttribArray(pos_id);
		glVertexAttribPointer(pos_id, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex2),
			reinterpret_cast<const GLvoid*>(offsetof(ParticleVertex2, x)));
		// bind color position
		auto color_id = particle_program->attr(Renderer::at_color);
		glEnableVertexAttribArray(color_id);
		glVertexAttribPointer(color_id, 4, GL_FLOAT, GL_FALSE, sizeof(ParticleVertex2),
			reinterpret_cast<const GLvoid*>(offsetof(ParticleVertex2, r)));

		glDrawArrays(GL_POINTS, 0, particle_count);

#ifdef _DEBUG
		glDisableVertexAttribArray(pos_id);
		glDisableVertexAttribArray(color_id);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
#endif
	}

	void RenderLayer2::render_text(Renderer2* renderer, const AABB2& camera_bb)
	{
		Matrix4 mat;
		mat.identity();
		for (const auto& text : texts)
		{
			// TODO: boundary check
			text->render(renderer, mat);
		}
	}

	void RenderLayer2::render_effects(Renderer2* renderer, const AABB2& camera_bb)
	{
		for (const auto& fx : effects)
		{
			fx->render(renderer, camera_bb);
		}
	}
}