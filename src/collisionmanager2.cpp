#include "stdafx.h"
#include <dukat/collisionmanager2.h>
#include <dukat/mathutil.h>

namespace dukat
{
	MemoryPool<CollisionManager2::Body> CollisionManager2::Body::_pool(1024);

	static std::vector<CollisionManager2::Body*> candidates;
	const CollisionManager2::predicate CollisionManager2::pred_active = [](CollisionManager2::Body* b) { return b->active; };
	const CollisionManager2::predicate CollisionManager2::pred_solid = [](CollisionManager2::Body* b) { return b->active && b->solid; };
	const CollisionManager2::predicate CollisionManager2::pred_sensor = [](CollisionManager2::Body* b) { return b->active && !b->solid; };
	const CollisionManager2::predicate CollisionManager2::pred_static = [](CollisionManager2::Body* b) { return b->active && b->solid && !b->dynamic; };
	const CollisionManager2::predicate CollisionManager2::pred_dynamic = [](CollisionManager2::Body* b) { return b->active && b->solid && b->dynamic; };

	CollisionManager2::CollisionManager2(GameBase* game) : Manager(game), 
		world_origin({ 0,0 }), world_size(1000.0f), world_depth(5), generation(0)
	{
		create_tree();
	}

	CollisionManager2::Body* CollisionManager2::create_body(bool dynamic)
	{
		static uint16_t last_id = 0;
		auto body = std::make_unique<Body>(last_id++);
		body->dynamic = dynamic;
		bodies.push_back(std::move(body));
		auto ptr = bodies.back().get();
		trigger(Message{ Events::BodyCreated, ptr, nullptr });
		return ptr;
	}

	void CollisionManager2::destroy_body(Body* body)
	{
		invalidate_contacts(body);

		// Remove from tree
		tree->remove(body);

		auto it = std::find_if(bodies.begin(), bodies.end(), 
			[body](const std::unique_ptr<Body>& b) { return body == b.get(); });
		if (it != bodies.end())
		{
			trigger(Message{ Events::BodyDestroyed, (*it).get(), nullptr });
			bodies.erase(it);
		}
	}

	void CollisionManager2::invalidate_contacts(Body* body)
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
	}

	void CollisionManager2::test_collision(Body* this_body, Body* other_body)
	{
		if (this_body == other_body)
			return;
		if (!this_body->dynamic && !other_body->dynamic)
			return; // static bodies do not collide with one another
		if (this_body->id > other_body->id)
			std::swap(this_body, other_body); // enforce body order stays consistent across checks

		// Check if collision has already been detected during this frame
		const auto id = hash(this_body, other_body);
		const auto contact_exists = contacts.contains(id);
		if (contact_exists && contacts[id].generation == generation)
			return;

		perfc.inc(PerformanceCounter::BB_CHECKS);
		Contact c;
		if (this_body->bb.intersect(other_body->bb, c.collision))
		{
			// Update contact if this is an existing collision
			if (contact_exists)
			{
				auto& old_contact = contacts[id];
				old_contact.generation = generation;
				old_contact.collision = c.collision;
				old_contact.body1 = this_body; // order of bodies can flip, so update them
				old_contact.body2 = other_body;
				old_contact.age++;
			}
			// Otherwise, create a new contact
			else
			{
				c.body1 = this_body;
				c.body2 = other_body;
				c.generation = generation;
				c.age = 0;
				contacts[id] = c;

				if (c.body1->owner != nullptr)
					c.body1->owner->trigger(Message{ Events::CollisionBegin, c.body2, &c });
				if (c.body2->owner != nullptr)
					c.body2->owner->trigger(Message{ Events::CollisionBegin, c.body1, &c });
			}
		}
	}

	void CollisionManager2::resolve_collisions(void)
	{
		for (auto it = contacts.begin(); it != contacts.end(); )
		{
			// clean up contacts which are no longer active
			if (it->second.generation != generation)
			{
				if (it->second.body1->owner != nullptr)
					it->second.body1->owner->trigger(Message{ Events::CollisionEnd, it->second.body2 });
				if (it->second.body2->owner != nullptr)
					it->second.body2->owner->trigger(Message{ Events::CollisionEnd, it->second.body1 });
				it = contacts.erase(it);
			}
			// attempt to resolve active contacts
			else
			{
				auto b1 = it->second.body1;
				auto b2 = it->second.body2;
				if (b1->solid && b2->solid)
				{
					auto shift = it->second.collision.delta;
					if (shift.x == 0.0f && shift.y == 0.0f)
						shift.x = 0.25f; // centers occupy the same position, force horizontal shift
					if (b1->dynamic)
					{
						const auto mfactor = b2->dynamic ? 1.0f - (b1->mass / (b1->mass + b2->mass)) : 1.0f;
						const auto b1_shift = shift * mfactor;
						b1->bb += b1_shift;
						if (b1->owner != nullptr)
							b1->owner->trigger(Message{ Events::CollisionResolve, &b1_shift, b2 });
					}
					if (b2->dynamic)
					{
						const auto mfactor = b1->dynamic ? 1.0f - (b2->mass / (b1->mass + b2->mass)) : 1.0f;
						const auto b2_shift = -shift * mfactor;
						b2->bb += b2_shift;
						if (b2->owner != nullptr)
							b2->owner->trigger(Message{ Events::CollisionResolve, &b2_shift, b1 });
					}
				}

				++it;
			}
		}
	}

	void CollisionManager2::update(float delta)
	{
		// broad phase - determine all possible collisions
		tree->clear();
		for (const auto& b : bodies)
		{
			if (b->active)
			{
				perfc.inc(PerformanceCounter::BODIES);
				tree->insert(b.get());
			}
		}

		// narrow phase - build up set of actual collisions
		std::queue<QuadTree<Body>*> nodes;
		for (const auto& b : bodies)
		{
			if (!b->active)
				continue;

			// Start with root, compare with each child
			nodes.push(tree.get());
			auto this_body = b.get();
			while (!nodes.empty())
			{
				auto t = nodes.front();
				auto idx = t->get_index(this_body);
				if (idx > -1 && t->has_child(idx))
				{
					nodes.push(t->child(idx));
				}
				const auto& values = t->get_values();
				for (auto& it : values)
				{
					test_collision(this_body, it);
				}
				nodes.pop();
			}
		}

		resolve_collisions();

		generation++;
	}

	void CollisionManager2::create_tree(void)
	{
		Vector2 dim{ 0.5f * world_size, 0.5f * world_size };
		tree = std::make_unique<QuadTree<Body>>(world_origin - dim, world_origin + dim, world_depth);
	}

	bool CollisionManager2::has_contact(Body* b) const
	{
		for (const auto& it : contacts)
		{
			if (it.second.body1 == b || it.second.body2 == b)
				return true;
		}
		return false;
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

	// Index direction flags
	static constexpr auto dir_left = 1;
	static constexpr auto dir_right = 2;
	static constexpr auto dir_top = 4;
	static constexpr auto dir_bottom = 8;

	int get_index_flags(const QuadTree<CollisionManager2::Body>& t, CollisionManager2::Body* body)
	{
		auto res = dir_left | dir_right | dir_top | dir_bottom;
		if (body->bb.max().x < t.center.x)
			res &= ~dir_right; // value is in left quadrants
		else if (body->bb.min().x >= t.center.x)
			res &= ~dir_left; // value is in right quadrants
		if (body->bb.max().y < t.center.y)
			res &= ~dir_bottom; // value is in top quadrants
		else if (body->bb.min().y >= t.center.y)
			res &= ~dir_top; // value is in bottom quadrants
		return res;
	}

	void CollisionManager2::collect_collisions(const QuadTree<Body>& t, Body* body, std::vector<Body*>& res) const
	{
		const auto flags = get_index_flags(t, body);
		if (flags & dir_right)
		{
			if ((flags & dir_top) != 0 && t.has_child(0))
				collect_collisions(*t.child(0), body, res); // check top-right
			if ((flags & dir_bottom) != 0 && t.has_child(1))
				collect_collisions(*t.child(1), body, res); // check bottom-right
		}
		if (flags & dir_left)
		{
			if ((flags & dir_top) != 0 && t.has_child(3))
				collect_collisions(*t.child(3), body, res); // check top-left
			if ((flags & dir_bottom) != 0 && t.has_child(2))
				collect_collisions(*t.child(2), body, res); // check bottom-left
		}
		const auto& values = t.get_values();
		res.insert(res.end(), values.begin(), values.end());
	}

	std::list<CollisionManager2::Body*> CollisionManager2::find(const Vector2& pos, const predicate& p) const
	{
		Body b{ 0 };
		b.bb = AABB2{ pos, pos };
		candidates.clear();
		collect_collisions(*tree, &b, candidates);

		std::list<Body*> res;
		for (Body* b : candidates)
		{
			if ((p == nullptr || p(b)) && b->bb.contains(pos))
				res.push_back(b);
		}
		return res;
	}

	std::list<CollisionManager2::Body*> CollisionManager2::find(const AABB2& bb, const predicate& p) const
	{
		Body b{ 0 };
		b.bb = bb;
		candidates.clear();
		collect_collisions(*tree, &b, candidates);

		std::list<Body*> res;
		for (Body* b : candidates)
		{
			if ((p == nullptr || p(b)) && b->bb.overlaps(bb))
				res.push_back(b);
		}
		return res;
	}

	void CollisionManager2::find(QuadTree<Body>* tree, const Ray2& ray, float min_t, float max_t, std::list<Body*>& list, const predicate& p) const
	{
		if (tree == nullptr)
			return;

		const AABB2 bb{ tree->min_v, tree->max_v };
		const auto tree_t = bb.intersect_ray(ray, min_t, max_t);
		if (tree_t == no_intersection)
			return; // no intersection with tree

		// check values at this level
		for (const auto& it : tree->get_values())
		{
			if (p == nullptr || p(it))
			{
				const auto t = it->bb.intersect_ray(ray, min_t, max_t);
				if (t != no_intersection)
					list.push_back(it);
			}
		}

		// check lower levels of the tree
		for (auto i = 0; i < 4; i++)
			find(tree->child(i), ray, min_t, max_t, list, p);
	}

	std::list<CollisionManager2::Body*> CollisionManager2::find(const Ray2& ray, float min_t, float max_t, const predicate& p) const
	{
		std::list<Body*> res;
		find(tree.get(), ray, min_t, max_t, res, p);
		return res;
	}

	CollisionManager2::Body* CollisionManager2::find_closest(QuadTree<Body>* tree, const Ray2& ray, float min_t, float max_t, float& t, const predicate& p) const
	{
		if (tree == nullptr)
			return nullptr;

		const AABB2 bb{ tree->min_v, tree->max_v };
		const auto tree_t = bb.intersect_ray(ray, min_t, max_t);
		if (tree_t == no_intersection)
			return nullptr; // no intersection with tree

		// check values at this level
		t = max_t;
		Body* best_body = nullptr;
		for (const auto& it : tree->get_values())
		{
			if (p == nullptr || p(it))
			{
				const auto body_t = it->bb.intersect_ray(ray, min_t, max_t);
				if (body_t < t)
				{
					t = body_t;
					best_body = it;
				}
			}
		}

		// check lower levels of the tree
		for (auto i = 0; i < 4; i++)
		{
			float child_t;
			auto res = find_closest(tree->child(i), ray, min_t, t, child_t, p);
			if (res != nullptr && child_t < t)
			{
				best_body = res;
				t = child_t;
			}
		}

		return best_body;
	}

	CollisionManager2::Body* CollisionManager2::find_closest(const Ray2& ray, float min_t, float max_t, float& t, const predicate& p) const
	{
		return find_closest(tree.get(), ray, min_t, max_t, t, p);
	}
}