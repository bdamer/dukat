#include "stdafx.h"
#include "collisionmanager2.h"
#include "debugeffect2.h"
#include "log.h"

namespace dukat
{
	static std::vector<CollisionManager2::Body*> candidates;

	CollisionManager2::CollisionManager2(GameBase* game) : Manager(game), 
		world_origin({ 0,0 }), world_size(1000.0f), world_depth(5)
	{
		create_tree();
	}

	CollisionManager2::Body* CollisionManager2::create_body(bool dynamic)
	{
		static uint16_t last_id = 0;
		auto body = std::make_unique<Body>(last_id++);
		body->dynamic = dynamic;
		bodies.push_back(std::move(body));
		return bodies.back().get();
	}

	void CollisionManager2::destroy_body(Body* body)
	{
		// Remove any contacts this body is part of
		for (const auto& c : get_contacts(body))
		{
			auto other_body = c->body1 == body ? c->body2 : c->body1;
			if (other_body->owner != nullptr)
			{
				other_body->owner->trigger(Message{ Events::CollisionEnd, body });
			}
			contacts.erase(hash(c->body1, c->body2));
		}

		auto it = std::find_if(bodies.begin(), bodies.end(), 
			[body](const std::unique_ptr<Body>& b) { return body == b.get(); });
		if (it != bodies.end())
		{
			bodies.erase(it);
		}
	}

	void CollisionManager2::find_collisions(const QuadTree<Body>& t, Body* body, std::vector<Body*>& res) const
	{
		auto idx = t.get_index(body);
		if (idx > -1 && t.has_child(idx))
		{
			find_collisions(*t.child(idx), body, res);
		}
		const auto& values = t.get_values();
		res.insert(res.end(), values.begin(), values.end());
	}

	void CollisionManager2::update(float delta)
	{
		contacts.clear();

		// broad phase - determine all possible collisions
		tree->clear();
		for (const auto& b : bodies)
		{
			if (b->active)
			{
				tree->insert(b.get());
			}
		}

		// narrow phase - build up set of actual collisions
		for (const auto& b : bodies)
		{
			candidates.clear();
			auto this_body = b.get();
			find_collisions(*tree, this_body, candidates);
			for (auto other_body : candidates)
			{
				if (this_body == other_body)
					continue;
				if (!this_body->dynamic && !other_body->dynamic)
					continue; // static bodies do not collide with one another

				// Check if collision has already been detected during this frame
				auto id = hash(this_body, other_body);
				if (contacts.count(id) > 0)
					continue;

				perfc.inc(PerformanceCounter::BB_CHECKS);
				Contact c;
				if (this_body->bb.intersect(other_body->bb, c.collision))
				{
					c.body1 = this_body;
					c.body2 = other_body;
					contacts[id] = c;

					if (c.body1->owner != nullptr)
						c.body1->owner->trigger(Message{ Events::CollisionBegin, c.body2, &c });
					if (c.body2->owner != nullptr)
						c.body2->owner->trigger(Message{ Events::CollisionBegin, c.body1, &c });
				}
			}
		}

		// attempt to resolve contacts
		for (const auto& it : contacts)
		{
			auto b1 = it.second.body1;
			auto b2 = it.second.body2;
			if (!b1->solid || !b2->solid)
				continue;
			auto shift = it.second.collision.delta;
			if (b1->dynamic)
			{
				b1->bb.min += shift;
				b1->bb.max += shift;
				if (b1->owner != nullptr)
					b1->owner->trigger(Message{ Events::CollisionResolve, &shift });
			}
			if (b2->dynamic)
			{
				shift = -shift;
				b2->bb.min += shift;
				b2->bb.max += shift;
				if (b2->owner != nullptr)
					b2->owner->trigger(Message{ Events::CollisionResolve, &shift });
			}
		}
	}

	void CollisionManager2::create_tree(void)
	{
		Vector2 dim{ 0.5f * world_size, 0.5f * world_size };
		tree = std::make_unique<QuadTree<Body>>(world_origin - dim, world_origin + dim, world_depth);
	}

	std::list<CollisionManager2::Contact*> CollisionManager2::get_contacts(Body* b) const
	{
		std::list<Contact*> res;
		for (const auto& it : contacts)
		{
			if (it.second.body1 == b || it.second.body2 == b)
				res.push_back(const_cast<Contact*>(&it.second));
		}
		return res;
	}

	std::list<CollisionManager2::Body*> CollisionManager2::get_bodies(const Vector2& p) const
	{
		Body b{ 0 };
		b.bb.min = b.bb.max = p;
		candidates.clear();
		find_collisions(*tree, &b, candidates);

		std::list<Body*> res;
		for (Body* b : candidates)
		{
			if (b->bb.contains(p))
			{
				res.push_back(b);
			}
		}

		return res;
	}
}