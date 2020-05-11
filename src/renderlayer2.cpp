#include "stdafx.h"
#include <dukat/renderlayer2.h>
#include <dukat/aabb2.h>
#include <dukat/bit.h>
#include <dukat/buffers.h>
#include <dukat/camera2.h>
#include <dukat/effect2.h>
#include <dukat/matrix4.h>
#include <dukat/particle.h>
#include <dukat/perfcounter.h>
#include <dukat/shadercache.h>
#include <dukat/sprite.h>
#include <dukat/textmeshinstance.h>
#include <dukat/renderer2.h>
#include <dukat/vertextypes2.h>

namespace dukat
{
	typedef Vertex2PSRC PVertex;

	// module-global buffer for particle data used during rendering
	static PVertex particle_data[Renderer2::max_particles];

	RenderLayer2::RenderLayer2(ShaderCache* shader_cache, VertexBuffer* sprite_buffer, VertexBuffer* particle_buffer,
	    const std::string& id, float priority, float parallax) : render_target(nullptr), composite_binder(nullptr),
		 sprite_buffer(sprite_buffer), particle_buffer(particle_buffer), is_visible(true),
		id(id), parallax(parallax), priority(priority), stage(Composite)
	{
		sprite_program = shader_cache->get_program("sc_sprite.vsh", "sc_sprite.fsh");
		particle_program = shader_cache->get_program("sc_particle.vsh", "sc_particle.fsh");
		composite_program = shader_cache->get_program("fx_default.vsh", "fx_default.fsh");
	}

	RenderLayer2::~RenderLayer2(void)
	{
	}

	void RenderLayer2::add(Sprite* sprite)
	{
		if (std::find(sprites.begin(), sprites.end(), sprite) != sprites.end())
			return; // sprite already added to this layer
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

		if (has_effects() && renderer->is_render_effects())
		{
			render_effects(renderer, camera_bb);
		}
		if (has_sprites() && renderer->is_render_sprites())
		{
			render_sprites(renderer, camera_bb);
		}
		if (has_particles() && renderer->is_render_particles())
		{
			render_particles(renderer, camera_bb);
		}
		if (has_text() && renderer->is_render_text())
		{
			render_text(renderer, camera_bb);
		}
	}

	void RenderLayer2::clear(void)
	{
		sprites.clear();
		effects.clear();
		particles.clear();
		texts.clear();
	}

	void RenderLayer2::fill_sprite_queue(const AABB2& camera_bb, std::function<bool(Sprite*)> predicate,
		std::priority_queue<Sprite*, std::deque<Sprite*>, SpriteComparator>& queue)
	{
		// Fill queue with sprites ordered by priority from low to high
		for (auto sprite : sprites)
		{
			if (predicate != nullptr && !predicate(sprite))
				continue; // exclude sprites which fail predicate

			if (check_flag(sprite->flags, Sprite::relative))
			{
				// TODO: perform occlusion check against untranslated camera bounding box
				sprite->flags |= Sprite::rendered;
				queue.push(sprite);
			}
			else
			{
				Vector2 half_dim(sprite->scale * sprite->w / 2.0f, sprite->scale * sprite->h / 2.0f);
				auto min_p = sprite->p - half_dim;
				auto max_p = sprite->p + half_dim;
				const auto center = sprite->flags & (Sprite::align_bottom | Sprite::align_left | Sprite::align_right | Sprite::align_top);
				if (center > 0)
				{
					if (check_flag(center, Sprite::align_bottom))
					{
						min_p.y -= (sprite->h / 2) * sprite->scale;
						max_p.y -= (sprite->h / 2) * sprite->scale;
					}
					else if (check_flag(center, Sprite::align_top))
					{
						min_p.y += (sprite->h / 2) * sprite->scale;
						max_p.y += (sprite->h / 2) * sprite->scale;
					}
					if (check_flag(center, Sprite::align_right))
					{
						min_p.x -= (sprite->w / 2) * sprite->scale;
						max_p.x -= (sprite->w / 2) * sprite->scale;
					}
					else if (check_flag(center, Sprite::align_left))
					{
						min_p.x += (sprite->w / 2) * sprite->scale;
						max_p.x += (sprite->w / 2) * sprite->scale;
					}
				}

				AABB2 sprite_bb(min_p, max_p);
				if (!camera_bb.overlaps(sprite_bb))
				{
					sprite->flags &= ~Sprite::rendered;
				}
				else
				{
					sprite->flags |= Sprite::rendered;
					queue.push(sprite);
				}
			}
		}

		perfc.inc(PerformanceCounter::SPRITES, queue.size());
		perfc.inc(PerformanceCounter::SPRITES_TOTAL, sprites.size());
	}

	void RenderLayer2::render_sprites(Renderer2* renderer, const AABB2& camera_bb, std::function<bool(Sprite*)> predicate)
	{
		static std::priority_queue<Sprite*, std::deque<Sprite*>, SpriteComparator> queue;
		fill_sprite_queue(camera_bb, predicate, queue);
		if (queue.empty())
			return; // nothing to render

		renderer->switch_shader(sprite_program);

		// Set parallax value for this layer
		glUniform1f(sprite_program->attr("u_parallax"), parallax);

		// bind sprite vertex buffers
#if OPENGL_VERSION >= 30
		glBindVertexArray(sprite_buffer->vao);
		glBindBuffer(GL_ARRAY_BUFFER, sprite_buffer->buffers[0]);

		// bind vertex position
		auto pos_id = sprite_program->attr(Renderer::at_pos);
		glEnableVertexAttribArray(pos_id);
		glVertexAttribPointer(pos_id, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2PT),
			reinterpret_cast<const GLvoid*>(offsetof(Vertex2PT, px)));
		// bind texture position
		auto uv_id = sprite_program->attr(Renderer::at_texcoord);
		glEnableVertexAttribArray(uv_id);
		glVertexAttribPointer(uv_id, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2PT),
			reinterpret_cast<const GLvoid*>(offsetof(Vertex2PT, tu)));
#else
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, sprite_buffer->buffers[0]);

		// bind vertex position
		glVertexPointer(2, GL_FLOAT, sizeof(Vertex2PT),
			reinterpret_cast<const GLvoid*>(offsetof(Vertex2PT, px)));
		// bind texture position
		glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex2PT),
			reinterpret_cast<const GLvoid*>(offsetof(Vertex2PT, tu)));
#endif

		// set texture unit 0 
		glUniform1i(sprite_program->attr(Renderer::uf_tex0), 0);
		
		// Get uniforms that will be set for each sprite
		auto uvwh_id = sprite_program->attr("u_uvwh");
		auto color_id = sprite_program->attr(Renderer::uf_color);
		auto model_id = sprite_program->attr(Renderer::uf_model);

		// Render in order
		GLuint last_texture = -1;
		Matrix4 mat_m;
		while (!queue.empty())
		{
			// get top element from queue
			auto sprite = queue.top();
			queue.pop();

			// switch texture if necessary
			if (last_texture != sprite->texture_id)
			{
				perfc.inc(PerformanceCounter::TEXTURES);

				// glActiveTextureARB(GL_TEXTURE0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, sprite->texture_id);
				last_texture = sprite->texture_id;
			}

			compute_model_matrix(*sprite, renderer->get_camera()->transform.position, mat_m);
			glUniformMatrix4fv(model_id, 1, false, &mat_m.m[0]);
			glUniform4fv(color_id, 1, &sprite->color.r);
			glUniform4fv(uvwh_id, 1, sprite->tex);
			
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

#ifdef _DEBUG
	#if OPENGL_VERSION >= 30
		// unbind buffers
		glDisableVertexAttribArray(pos_id);
		glDisableVertexAttribArray(uv_id);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	#else
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	#endif
#endif
	}

	void RenderLayer2::compute_model_matrix(const Sprite& sprite, const Vector2& camera_position, Matrix4& mat_model)
	{
		Vector2 pos = sprite.p;

		// adjust position based on alignment
		const auto center = sprite.flags & (Sprite::align_bottom | Sprite::align_left | Sprite::align_right | Sprite::align_top);
		if (center > 0)
		{
			if (check_flag(center, Sprite::align_bottom))
			{
				pos.y -= (sprite.h / 2) * sprite.scale;
			}
			else if (check_flag(center, Sprite::align_top))
			{
				pos.y += (sprite.h / 2) * sprite.scale;
			}
			if (check_flag(center, Sprite::align_right))
			{
				pos.x -= (sprite.w / 2) * sprite.scale;
			}
			else if (check_flag(center, Sprite::align_left))
			{
				pos.x += (sprite.w / 2) * sprite.scale;
			}
		}

		// if we're in relative addressing mode, transpose sprite
		// position by camera position.
		if (check_flag(sprite.flags, Sprite::relative))
			pos += camera_position;

		// scale * rotation * translation
		static Matrix4 tmp;
		mat_model.setup_translation(Vector3(pos.x, pos.y, 0.0f));
		if (sprite.rot != 0.0f)
		{
			tmp.setup_rotation(Vector3::unit_z, sprite.rot);
			mat_model *= tmp;
		}
		tmp.setup_scale(Vector3(sprite.scale * sprite.w, sprite.scale * sprite.h, 1.0f));
		mat_model *= tmp;
	}

	void RenderLayer2::render_particles(Renderer2* renderer, const AABB2& camera_bb)
	{
		// increase camera bb slightly to avoid culling particles with size > 1
		// which fall just outside of screen rect; otherwise these will cause flickering
		const Vector2 padding{ 4, 4 };
		const auto bb = AABB2{ camera_bb.min - padding, camera_bb.max + padding };
		auto particle_count = 0;
		for (auto it = particles.begin(); it != particles.end(); )
		{
			Particle* p = (*it);
			// remove dead particles
			if (p->ttl <= 0)
			{
				it = particles.erase(it);
				continue;
			}

			// check if particle visible and store result in ->rendered
			if (bb.contains(p->pos))
			{
				p->flags |= Particle::Rendered;
				particle_data[particle_count].px = p->pos.x;
				particle_data[particle_count].py = p->pos.y;
				particle_data[particle_count].size = p->size;
				particle_data[particle_count].ry = p->ry;
				particle_data[particle_count].cr = p->color.r;
				particle_data[particle_count].cg = p->color.g;
				particle_data[particle_count].cb = p->color.b;
				particle_data[particle_count].ca = p->color.a;
				particle_count++;
			}
			else
			{
				p->flags &= ~Particle::Rendered;
			}
			++it;
		}
		perfc.inc(PerformanceCounter::PARTICLES, particle_count);

		if (particle_count == 0)
		{
			return; // no particles left to render
		}

		renderer->switch_shader(particle_program);

		// Set parallax value for this layer
		glUniform1f(particle_program->attr("u_parallax"), parallax);

#if OPENGL_VERSION >= 30
		// bind particle vertex buffers
		glBindVertexArray(particle_buffer->vao);
		glBindBuffer(GL_ARRAY_BUFFER, particle_buffer->buffers[0]);
		// Orphan buffer to improve streaming performance
		glBufferData(GL_ARRAY_BUFFER, Renderer2::max_particles * sizeof(PVertex), nullptr, GL_STREAM_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, particle_count * sizeof(PVertex), particle_data);
		// bind vertex position
		auto pos_id = particle_program->attr(Renderer::at_pos);
		glEnableVertexAttribArray(pos_id);
		glVertexAttribPointer(pos_id, 4, GL_FLOAT, GL_FALSE, sizeof(PVertex),
			reinterpret_cast<const GLvoid*>(offsetof(PVertex, px)));
		// bind color position
		auto color_id = particle_program->attr(Renderer::at_color);
		glEnableVertexAttribArray(color_id);
		glVertexAttribPointer(color_id, 4, GL_FLOAT, GL_FALSE, sizeof(PVertex),
			reinterpret_cast<const GLvoid*>(offsetof(PVertex, cr)));
#else
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, particle_buffer->buffers[0]);

		// Orphan buffer to improve streaming performance
		glBufferData(GL_ARRAY_BUFFER, Renderer2::max_particles * sizeof(PVertex), NULL, GL_STREAM_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, particle_count * sizeof(PVertex), particle_data);

		// bind vertex position
		glVertexPointer(4, GL_FLOAT, sizeof(PVertex),
			reinterpret_cast<const GLvoid*>(offsetof(PVertex, px)));
		// bind color position
		glColorPointer(4, GL_FLOAT, sizeof(PVertex),
			reinterpret_cast<const GLvoid*>(offsetof(PVertex, cr)));
#endif

		glDrawArrays(GL_POINTS, 0, particle_count);

#ifdef _DEBUG
	#if OPENGL_VERSION >= 30
		glDisableVertexAttribArray(pos_id);
		glDisableVertexAttribArray(color_id);
		glBindVertexArray(0);
	#else
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
	#endif
		glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
	}

	void RenderLayer2::render_text(Renderer2* renderer, const AABB2& camera_bb)
	{
		// set up matrix once
		Matrix4 mat_identity;
		mat_identity.identity();
		for (const auto& text : texts)
		{
			if (text->visible)
			{
				// TODO: perform boundary check
				text->render(renderer, mat_identity);
			}
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