#include "stdafx.h"
#include <dukat/debugeffect2.h>
#include <dukat/vertextypes2.h>
#include <dukat/mathutil.h>
#include <dukat/collisionmanager2.h>

namespace dukat
{
	static std::vector<Vertex2P> buffer;

	DebugEffect2::DebugEffect2(Game2* game, float scale) : scale(scale), game(game), flags(static_cast<Flags>(Flags::BODIES | Flags::GRID))
	{
		program = game->get_shaders()->get_program("fx_debug.vsh", "fx_debug.fsh");
		std::vector<VertexAttribute> attr;
		attr.push_back(VertexAttribute(Renderer::at_pos, 2, offsetof(Vertex2P, px)));
		mesh = std::make_unique<MeshData>(GL_LINE_LOOP, 1024, 0, attr, true);
	}

	DebugEffect2::~DebugEffect2(void)
	{
	}

	void DebugEffect2::render_bounding_box(const AABB2& bb, const Color& color) const
	{
		render_rect(bb.min, bb.max, color);
	}

	void DebugEffect2::render_rect(const Vector2& min, const Vector2& max, const Color& color) const
	{
		program->set(Renderer::uf_color, color.r, color.g, color.b, color.a);
		buffer.resize(4);
		buffer[0].px = min.x * scale;
		buffer[0].py = min.y * scale;
		buffer[1].px = max.x * scale;
		buffer[1].py = min.y * scale;
		buffer[2].px = max.x * scale;
		buffer[2].py = max.y * scale;
		buffer[3].px = min.x * scale;
		buffer[3].py = max.y * scale;
		mesh->set_vertices(buffer.data(), 4);
		mesh->render(program);
	}

	void DebugEffect2::render_circle(const Vector2& center, float radius, const Color& color) const
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

	void DebugEffect2::render(Renderer2* renderer, const AABB2& camera_bb)
	{
		renderer->switch_shader(program);

		auto world_bb = camera_bb / scale;

		auto cm = game->get<CollisionManager2>();

		if ((flags & Flags::GRID) == Flags::GRID)
		{
			Color tree_color{ 0.0f, 0.0f, 1.0f, 1.0f };
			std::queue<QuadTree<CollisionManager2::Body>*> queue;
			queue.push(cm->tree.get());

			while (!queue.empty())
			{
				auto t = queue.front();
				queue.pop();

				for (auto i = 0; i < 4; i++)
				{
					if (t->has_child(i))
						queue.push(t->child(i));
				}

				if (world_bb.contains(t->min) || world_bb.contains(t->max))
					render_rect(t->min, t->max, tree_color);
			}
		}

		if ((flags & Flags::BODIES) == Flags::BODIES)
		{
			Color fixed_color{ 1.0f, 1.0f, 1.0f, 0.5f };
			Color dynamic_color{ 1.0f, 1.0f, 1.0f, 1.0f };
			Color sensor_color{ 1.0f, 1.0f, 0.0f, 1.0f };
			Color contact_color{ 1.0f, 0.0f, 0.0f, 0.8f };
			for (const auto& b : cm->bodies)
			{
				if (!world_bb.overlaps(b->bb))
					continue;

				if (!b->dynamic)
				{
					render_bounding_box(b->bb, fixed_color);
				}
				else if (!cm->get_contacts(b.get()).empty())
				{
					render_bounding_box(b->bb, contact_color);
				}
				else if (b->solid)
				{
					render_bounding_box(b->bb, dynamic_color);
				}
				else
				{
					render_bounding_box(b->bb, sensor_color);
				}
			}
		}
	}
}
