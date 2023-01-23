#pragma once

#include <deque>
#include <memory>
#include <string>
#include <vector>
#include <queue>

#ifndef OPENGL_VERSION
#include "version.h"
#endif // !OPENGL_VERSION

#include "bit.h"
#include "color.h"
#include "sprite.h"
#include "renderer.h"

namespace dukat
{
	class AABB2;
	class Camera2;
	class Effect2;
	class Matrix4;
	struct Particle;
	class Renderer2;
	class ShaderCache;
	class ShaderProgram;
	class TextMeshInstance;
	struct VertexBuffer;

	class RenderLayer2
	{
	public:
		enum Flags
		{
			Visible = 1,	// Show layer
			Relative = 2,	// Render relative to cmaera
			RenderFx = 4,	// Render effects
			RenderSprites = 8,	// Render sprites
			RenderParticles = 16,	// Render particles
			RenderText = 32	// Render text
		};

	private:
		ShaderProgram* sprite_program;
		ShaderProgram* particle_program;
		ShaderProgram* composite_program;
		std::function<void(ShaderProgram*)> composite_binder;
		std::unique_ptr<Texture> render_target;
		VertexBuffer* sprite_buffer;
		VertexBuffer* particle_buffer;
		std::vector<std::unique_ptr<Effect2>> effects;
		std::vector<Sprite*> sprites;
		std::deque<Particle*> particles;
		std::vector<TextMeshInstance*> texts;
		int render_flags;

		// Fills up priority queue with visible sprites.
		void fill_sprite_queue(const AABB2& camera_bb, std::function<bool(Sprite*)> predicate,
			std::priority_queue<Sprite*, std::deque<Sprite*>, SpriteComparator<Sprite*>>& queue);
		void bind_sprite_buffers(GLint pos_id, GLint uv_id);
		void unbind_sprite_buffers(GLint pos_id, GLint uv_id);
		// Generates sprite model matrix.
		void compute_model_matrix(const Sprite& sprite, const Vector2& camera_position, float camera_mag, Matrix4& mat_model);
		std::size_t fill_particle_queue(Camera2* cam, const AABB2& camera_bb);

	public:
		const std::string id;
		// value for parallax projection
		const float parallax;
		// base rendering priority
		const float priority;	

		// Constructor
		RenderLayer2(ShaderCache* shader_cache, VertexBuffer* sprite_buffer, VertexBuffer* particle_buffer,
			const std::string& id, float priority, float parallax = 1.0f);
		~RenderLayer2(void);

		bool has_effects(void) const { return !effects.empty(); }
		bool has_sprites(void) const { return !sprites.empty(); }
		bool has_particles(void) const { return !particles.empty(); }
		bool has_text(void) const { return !texts.empty(); }

		Effect2* add(std::unique_ptr<Effect2> fx);
		void remove(Effect2* fx);
		void add(Sprite* sprite);
		void remove(Sprite* sprite);
		void add(Particle* p);
		void remove(Particle* p);
		void add(TextMeshInstance* text);
		void remove(TextMeshInstance* text);
		
		void render(Renderer2* renderer);
		void render_effects(Renderer2* renderer, const AABB2& camera_bb);
		void render_sprites(Renderer2* renderer, const AABB2& camera_bb, std::function<bool(Sprite*)> predicate = nullptr);
		void render_particles(Renderer2* renderer, const AABB2& camera_bb);
		void render_text(Renderer2* renderer, const AABB2& camera_bb);

		void show(void) { render_flags |= Flags::Visible; }
		void hide(void) { render_flags &= ~Flags::Visible; }
		bool visible(void) const { return (render_flags & Flags::Visible) == Flags::Visible; }
		bool relative(void) const { return (render_flags & Flags::Relative) == Flags::Relative; }
		// Removes all renderables from this layer.
		void clear(void);

		ShaderProgram* get_sprite_program(void) const { return sprite_program; }
		void set_sprite_program(ShaderProgram* sprite_program) { this->sprite_program = sprite_program; }
		ShaderProgram* get_particle_program(void) const { return particle_program; }
		void set_particle_program(ShaderProgram* particle_program) { this->particle_program = particle_program; }
		ShaderProgram* get_composite_program(void) const { return composite_program; }
		void set_composite_program(ShaderProgram* composite_program) { this->composite_program = composite_program; }
		const std::function<void(ShaderProgram*)>& get_composite_binder(void) const { return composite_binder; }
		void set_composite_binder(std::function<void(ShaderProgram*)> binder) { composite_binder = binder; }
		Texture* get_render_target(void) const { return render_target != nullptr ? render_target.get() : nullptr; }
		void set_render_target(std::unique_ptr<Texture> render_target) { this->render_target = std::move(render_target); }
	
		// Sets flags
		void set_render_flag(int flag, bool val) { set_flag(render_flags, flag, val); }
	};
}