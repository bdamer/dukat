#pragma once

#include <deque>
#include <memory>
#include <string>
#include <vector>
#include <queue>

#include "color.h"
#include "renderlayer2.h"
#include "sprite.h"

namespace dukat
{
	class AABB2;
	class Camera2;
	class Effect;
	class Matrix4;
	struct Particle;
	class Renderer2;
	class ShaderCache;
	class ShaderProgram;
	struct VertexBuffer;

	class RenderLayer2
	{
	private:
		ShaderCache* shader_cache;
		ShaderProgram* sprite_program;
		ShaderProgram* particle_program;
		VertexBuffer* sprite_buffer;
		VertexBuffer* particle_buffer;
		std::vector<std::unique_ptr<Effect>> effects;
		std::vector<Sprite*> sprites;
		std::deque<Particle*> particles;
		bool is_visible;

		void render_effects(Renderer2* renderer, const AABB2& camera_bb);
		void render_sprites(Renderer2* renderer, const AABB2& camera_bb);
		void render_particles(Renderer2* renderer, const AABB2& camera_bb);

		// Fills up priority queue with visible sprites.
		void fill_sprite_queue(const AABB2& camera_bb, 
			std::priority_queue<Sprite*, std::deque<Sprite*>, SpriteComparator>& queue);

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

		Effect* add(std::unique_ptr<Effect> fx);
		void remove(Effect* fx);
		void add(Sprite* sprite);
		void remove(Sprite* sprite);
		void add(Particle* p);
		void remove(Particle* p);
		
		void render(Renderer2* renderer);
		void show(void) { is_visible = true; }
		void hide(void) { is_visible = false; }
		bool visible(void) const { return is_visible; }
	
		ShaderProgram* get_sprite_program(void) const { return sprite_program; }
		void set_sprite_program(ShaderProgram* sprite_program) { this->sprite_program = sprite_program; }
		ShaderProgram* get_particle_program(void) const { return particle_program; }
		void set_particle_program(ShaderProgram* particle_program) { this->particle_program = particle_program; }
	};
}