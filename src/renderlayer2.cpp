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
#include <dukat/sysutil.h>
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
		sprite_buffer(sprite_buffer), particle_buffer(particle_buffer), id(id), parallax(parallax), priority(priority), 
		render_flags(Visible | RenderFx | RenderSprites | RenderParticles | RenderText)
	{
		sprite_program = shader_cache->get_program("sc_sprite.vsh", "sc_sprite.fsh");
		particle_program = shader_cache->get_program("sc_particle.vsh", "sc_particle.fsh");
		composite_program = shader_cache->get_program("fx_default.vsh", "fx_default.fsh");
	}

	RenderLayer2::~RenderLayer2(void) { }

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

	void RenderLayer2::add(TextMeshInstance* text)
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
		const auto camera_bb = camera->get_bb(parallax);

		if (has_effects() && check_flag(render_flags, RenderFx) && renderer->is_render_effects())
			render_effects(renderer, camera_bb);
		if (has_sprites() && check_flag(render_flags, RenderSprites) && renderer->is_render_sprites())
			render_sprites(renderer, camera_bb);
		if (has_particles() && check_flag(render_flags, RenderParticles) && renderer->is_render_particles())
			render_particles(renderer, camera_bb);
		if (has_text() && check_flag(render_flags, RenderText) && renderer->is_render_text())
			render_text(renderer, camera_bb);
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
		const auto layer_relative = check_flag(render_flags, Relative);
		for (auto sprite : sprites)
		{
			if (predicate != nullptr && !predicate(sprite))
				continue; // exclude sprites which fail predicate

			if (layer_relative || check_flag(sprite->flags, Sprite::relative))
			{
				// TODO: perform occlusion check against untranslated camera bounding box
				queue.push(sprite);
			}
			else
			{
				const auto sprite_bb = compute_sprite_bb(*sprite);
				if (camera_bb.overlaps(sprite_bb))
					queue.push(sprite);
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

		// Get uniforms that will be set for each sprite
		const auto pos_id = sprite_program->attr(Renderer::at_pos);
		const auto uv_id = sprite_program->attr(Renderer::at_texcoord);
		const auto uvwh_id = sprite_program->attr("u_uvwh");
		const auto size_id = sprite_program->attr("u_size");
		const auto color_id = sprite_program->attr(Renderer::uf_color);
		const auto model_id = sprite_program->attr(Renderer::uf_model);

		bind_sprite_buffers(pos_id, uv_id);

		auto cam = renderer->get_camera();
		const auto& cam_pos = cam->transform.position;
		const auto cam_mag = cam->get_mag_factor();

		// Render in order
		GLuint last_texture = -1;
		Matrix4 mat_m;
		GLfloat uvwh[4];
		while (!queue.empty())
		{
			// get top element from queue
			auto sprite = queue.top();
			queue.pop();

			// switch texture if necessary
			if (last_texture != sprite->texture_id)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, sprite->texture_id);
				last_texture = sprite->texture_id;
				perfc.inc(PerformanceCounter::TEXTURES);
			}

			compute_model_matrix(*sprite, cam_pos, cam_mag, mat_m);
			glUniformMatrix4fv(model_id, 1, false, &mat_m.m[0]);
			glUniform4fv(color_id, 1, &sprite->color.r);
			if (size_id >= 0)
				glUniform2f(size_id, static_cast<float>(sprite->w), static_cast<float>(sprite->h));

			if (uvwh_id >= 0)
			{
				if ((sprite->flags & Sprite::flip_h) == 0)
				{
					uvwh[0] = sprite->tex[0];
					uvwh[2] = sprite->tex[2];
				}
				else
				{
					uvwh[0] = sprite->tex[0] + sprite->tex[2];
					uvwh[2] = -sprite->tex[2];
				}

				if ((sprite->flags & Sprite::flip_v) == 0)
				{
					uvwh[1] = sprite->tex[1];
					uvwh[3] = sprite->tex[3];
				}
				else
				{
					uvwh[1] = sprite->tex[1] + sprite->tex[3];
					uvwh[3] = -sprite->tex[3];
				}
				glUniform4fv(uvwh_id, 1, uvwh);
			}

			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		}

		unbind_sprite_buffers(pos_id, uv_id);
	}

	void RenderLayer2::bind_sprite_buffers(GLint pos_id, GLint uv_id)
	{
		// Set parallax value for this layer
		glUniform1f(sprite_program->attr("u_parallax"), parallax);

		// bind sprite vertex buffers
#if OPENGL_VERSION >= 30
		glBindVertexArray(sprite_buffer->vao);
		glBindBuffer(GL_ARRAY_BUFFER, sprite_buffer->buffers[0]);

		// bind vertex position
		glEnableVertexAttribArray(pos_id);
		glVertexAttribPointer(pos_id, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2PT),
			reinterpret_cast<const GLvoid*>(offsetof(Vertex2PT, px)));
		// bind texture position
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
	}

	void RenderLayer2::unbind_sprite_buffers(GLint pos_id, GLint uv_id)
	{
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
		gl_check_error();
#endif
	}

	void RenderLayer2::compute_model_matrix(const Sprite& sprite, const Vector2& camera_position, float camera_mag, Matrix4& mat_model)
	{
		auto pos = sprite.p;

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
		if (check_flag(render_flags, Flags::Relative) || check_flag(sprite.flags, Sprite::relative))
			pos += camera_position;

		// scale * rotation * translation
		static Matrix4 tmp;
		mat_model.setup_translation(Vector3(pos.x * camera_mag, pos.y * camera_mag, 0.0f));
		if (sprite.rot != 0.0f)
		{
			tmp.setup_rotation(Vector3::unit_z, sprite.rot);
			mat_model *= tmp;
		}
		tmp.setup_scale(Vector3(sprite.scale * sprite.w * camera_mag, sprite.scale * sprite.h * camera_mag, 1.0f));
		mat_model *= tmp;
	}

	std::size_t RenderLayer2::fill_particle_queue(Camera2* cam, const AABB2& camera_bb)
	{
		// increase camera bb slightly to avoid culling particles with size > 1
		// which fall just outside of screen rect; otherwise these will cause flickering
		const Vector2 padding{ 4, 4 };
		const auto layer_relative = check_flag(render_flags, Relative);
		const auto bb = AABB2{ camera_bb.min() - padding, camera_bb.max() + padding };

		const auto& cam_pos = cam->transform.position;
		const auto cam_mag = cam->get_mag_factor();

		auto particle_count = 0u;
		for (auto it = particles.begin(); it != particles.end(); )
		{
			auto p = (*it);
			// remove dead particles
			if (p->ttl <= 0)
			{
				it = particles.erase(it);
				continue;
			}

			// check if particle visible and store result in ->rendered
			if (layer_relative || bb.contains(p->pos))
			{
				p->flags |= Particle::Rendered;
				if (layer_relative)
				{
					particle_data[particle_count].px = p->pos.x + cam_pos.x;
					particle_data[particle_count].py = p->pos.y + cam_pos.y;
				}
				else
				{
					particle_data[particle_count].px = p->pos.x;
					particle_data[particle_count].py = p->pos.y;
				}
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
		return particle_count;
	}

	void RenderLayer2::render_particles(Renderer2* renderer, const AABB2& camera_bb)
	{
		auto cam = renderer->get_camera();
		const auto particle_count = fill_particle_queue(cam, camera_bb);
		if (particle_count == 0u)
			return; // no particles left to render

		renderer->switch_shader(particle_program);

		// Set parallax value for this layer
		particle_program->set("u_parallax", parallax);

		// Bind model matrix
		Matrix4 mat_m;
		mat_m.setup_scale(cam->get_mag_factor());
		particle_program->set_matrix4(Renderer::uf_model, mat_m);

#if OPENGL_VERSION >= 30
		// bind particle vertex buffers
		glBindVertexArray(particle_buffer->vao);
		glBindBuffer(GL_ARRAY_BUFFER, particle_buffer->buffers[0]);
		// Orphan buffer to improve streaming performance
		glBufferData(GL_ARRAY_BUFFER, Renderer2::max_particles * sizeof(PVertex), nullptr, GL_STREAM_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, particle_count * sizeof(PVertex), particle_data);
		// bind vertex position
		const auto pos_id = particle_program->attr(Renderer::at_pos);
		glEnableVertexAttribArray(pos_id);
		glVertexAttribPointer(pos_id, 4, GL_FLOAT, GL_FALSE, sizeof(PVertex),
			reinterpret_cast<const GLvoid*>(offsetof(PVertex, px)));
		// bind color position
		const auto color_id = particle_program->attr(Renderer::at_color);
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
		gl_check_error();
#endif
	}

	void RenderLayer2::render_text(Renderer2* renderer, const AABB2& camera_bb)
	{
		// set up matrix once
		Matrix4 mat_identity;
		const auto camera_mag = renderer->get_camera()->get_mag_factor();
		mat_identity.setup_scale(Vector3(camera_mag, camera_mag, 1.0f));
		for (const auto& text : texts)
		{
			if (text->visible)
				text->render(renderer, mat_identity);
		}
	}

	void RenderLayer2::render_effects(Renderer2* renderer, const AABB2& camera_bb)
	{
		for (const auto& fx : effects)
			fx->render(renderer, camera_bb);
	}
}
