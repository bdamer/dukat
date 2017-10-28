#pragma once

#include <deque>
#include <memory>
#include <string>
#include <vector>
#include <queue>

#ifndef OPENGL_VERSION
#include "version.h"
#endif // !OPENGL_VERSION

#include "color.h"
#include "renderlayer2.h"
#include "sprite.h"

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
	private:
		ShaderCache* shader_cache;
		ShaderProgram* sprite_program;
		ShaderProgram* particle_program;
		VertexBuffer* sprite_buffer;
		VertexBuffer* particle_buffer;
		std::vector<std::unique_ptr<Effect2>> effects;
		std::vector<Sprite*> sprites;
		std::deque<Particle*> particles;
		std::vector<TextMeshInstance*> texts;
		bool is_visible;

		void render_effects(Renderer2* renderer, const AABB2& camera_bb);
		void render_sprites(Renderer2* renderer, const AABB2& camera_bb);
		void render_particles(Renderer2* renderer, const AABB2& camera_bb);
		void render_text(Renderer2* renderer, const AABB2& camera_bb);

		// Fills up priority queue with visible sprites.
		void fill_sprite_queue(const AABB2& camera_bb, 
			std::priority_queue<Sprite*, std::deque<Sprite*>, SpriteComparator>& queue);

		// Generates sprite model matrix.
		void compute_model_matrix(const Sprite& sprite, const Vector2& camera_position, Matrix4& mat_model);

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
		void show(void) { is_visible = true; }
		void hide(void) { is_visible = false; }
		bool visible(void) const { return is_visible; }
		// Removes all renderables from this layer.
		void clear(void);

		ShaderProgram* get_sprite_program(void) const { return sprite_program; }
		void set_sprite_program(ShaderProgram* sprite_program) { this->sprite_program = sprite_program; }
		ShaderProgram* get_particle_program(void) const { return particle_program; }
		void set_particle_program(ShaderProgram* particle_program) { this->particle_program = particle_program; }
	};
}