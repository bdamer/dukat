#pragma once

#include "effect2.h"
#include "game2.h"
#include <Box2D/Box2D.h>

namespace dukat
{
	class DebugEffect2 : public Effect2, public b2Draw
	{
	private:
		static constexpr int circle_segments = 15;
		const float scale;

		Game2* game;
		b2World* world;
		ShaderProgram* program;
		std::unique_ptr<MeshData> mesh;

	public:
		DebugEffect2(Game2* game, float scale);
		~DebugEffect2(void);

		void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
		void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
		void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color);
		void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color);
		void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color);
		void DrawTransform(const b2Transform& xf);
		void DrawPoint(const b2Vec2 &, float32, const b2Color &);

		void render(Renderer2* renderer, const AABB2& camera_bb);
	};
}