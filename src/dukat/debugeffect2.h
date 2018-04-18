#pragma once

#include "effect2.h"
#include "game2.h"

namespace dukat
{
	struct Body;

	class DebugEffect2 : public Effect2
	{
	public:
		enum Flags
		{
			BODIES = 1,
			GRID = 2
		};

	private:
		static constexpr int circle_segments = 15;
		const float scale;

		Game2* game;
		ShaderProgram* program;
		std::unique_ptr<MeshData> mesh;
		Flags flags;

	public:
		DebugEffect2(Game2* game, float scale);
		~DebugEffect2(void);
		
		void render_bounding_box(const AABB2& bb, const Color& color) const;
		void render_rect(const Vector2& min, const Vector2& max, const Color& color) const;
		void render_circle(const Vector2& center, float radius, const Color& color) const;
		void render(Renderer2* renderer, const AABB2& camera_bb);

		void set_flags(Flags flags) { this->flags = flags; }
	};
}