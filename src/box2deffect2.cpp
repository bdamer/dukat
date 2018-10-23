#include "stdafx.h"
#include "box2deffect2.h"
#include "box2dmanager.h"
#include "vertextypes2.h"
#include "mathutil.h"

#ifdef BOX2D_SUPPORT

namespace dukat
{
	static std::vector<Vertex2P> buffer;

	Box2DEffect2::Box2DEffect2(Game2* game, float scale) : scale(scale)
	{
		program = game->get_shaders()->get_program("fx_debug.vsh", "fx_debug.fsh");
		world = game->get<Box2DManager>()->get_world();
		world->SetDebugDraw(this);
		SetFlags(b2Draw::e_shapeBit | b2Draw::e_aabbBit | b2Draw::e_centerOfMassBit);
	
		std::vector<VertexAttribute> attr;
		attr.push_back(VertexAttribute(Renderer::at_pos, 2, offsetof(Vertex2P, px)));
		mesh = std::make_unique<MeshData>(GL_LINE_LOOP, 1024, 0, attr, true);
	}

	Box2DEffect2::~Box2DEffect2(void)
	{
		world->SetDebugDraw(nullptr);
	}

	void Box2DEffect2::DrawPolygon(const b2Vec2* vertices, int32 vertex_count, const b2Color& color)
	{
		program->set(Renderer::uf_color, color.r, color.g, color.b, color.a);
		buffer.resize(vertex_count);
		for (auto i = 0; i < vertex_count; i++)
		{
			buffer[i].px = vertices[i].x * scale;
			buffer[i].py = vertices[i].y * scale;
		}
		mesh->set_vertices(buffer.data(), vertex_count);
		mesh->render(program);
	}

	void Box2DEffect2::DrawSolidPolygon(const b2Vec2* vertices, int32 vertex_count, const b2Color& color)
	{
		program->set(Renderer::uf_color, color.r, color.g, color.b, color.a);
		buffer.resize(vertex_count);
		for (auto i = 0; i < vertex_count; i++)
		{
			buffer[i].px = vertices[i].x * scale;
			buffer[i].py = vertices[i].y * scale;
		}
		mesh->set_vertices(buffer.data(), vertex_count);
		mesh->render(program);
	}

	void Box2DEffect2::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
	{
		program->set(Renderer::uf_color, color.r, color.g, color.b, color.a);
		buffer.resize(circle_segments);
		for (auto i = 0; i < circle_segments; i++)
		{
			auto theta = two_pi * static_cast<float>(i) / static_cast<float>(circle_segments);
			buffer[i].px = (center.x + radius * cosf(theta)) * scale;
			buffer[i].py = (center.y + radius * sinf(theta)) * scale;
		}
		mesh->set_vertices(buffer.data(), circle_segments);
		mesh->render(program);
	}

	void Box2DEffect2::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
	{
		program->set(Renderer::uf_color, color.r, color.g, color.b, color.a);
		buffer.resize(circle_segments);
		for (auto i = 0; i < circle_segments; i++)
		{
			auto theta = two_pi * static_cast<float>(i) / static_cast<float>(circle_segments);
			buffer[i].px = (center.x + radius * cosf(theta)) * scale;
			buffer[i].py = (center.y + radius * sinf(theta)) * scale;
		}
		mesh->set_vertices(buffer.data(), circle_segments);
		mesh->render(program);
	}

	void Box2DEffect2::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
	{
	}

	void Box2DEffect2::DrawTransform(const b2Transform& xf)
	{
	}

	void Box2DEffect2::DrawPoint(const b2Vec2&, float32, const b2Color&)
	{
	}

	void Box2DEffect2::render(Renderer2* renderer, const AABB2& camera_bb)
	{
		renderer->switch_shader(program);
		world->DrawDebugData();
	}
}

#endif