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
		Game2* game;
		Flags flags;
		std::unique_ptr<MeshData> mesh;

	protected:
		float scale;
		ShaderProgram* program;

		void render_bounding_box(const AABB2& bb, const Color& color) const;
		void render_rect(const Vector2& min, const Vector2& max, const Color& color) const;
		void render_circle(const Vector2& center, float radius, const Color& color) const;
		void render_line(const std::vector<Vector2>& segments, const Color& color) const;

	public:
		DebugEffect2(Game2* game);
		virtual ~DebugEffect2(void) override;
		
		virtual void render(Renderer2* renderer, const AABB2& camera_bb) override;

		void set_flags(Flags flags) { this->flags = flags; }
	};
}