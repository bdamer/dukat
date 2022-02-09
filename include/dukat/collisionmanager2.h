#pragma once

#include <list>
#include <memory>
#include <robin_hood.h>

#include "game2.h"
#include "manager.h"
#include "memorypool.h"
#include "messenger.h"
#include "quadtree.h"

namespace dukat
{
	class DebugEffect2;

	class CollisionManager2 : public Manager, public Messenger
	{
	public:	
		struct Body
		{
			const uint16_t id;
			bool dynamic;	// dynamic bodies can be moved as part of collision resolution
			bool solid;		// if true, will cause this body to take part in collision resolution
			bool active;	// if false, will cause this body to be ignored by collision manager
			float mass;		// Mass factor of this body use during collision resolution with other bodies
			AABB2 bb;
			Messenger* owner;

			Body(uint16_t id) : id(id), dynamic(true), solid(true), active(true), mass(1.0f), owner(nullptr) { }

			// Custom memory allocation
			static MemoryPool<Body> _pool;
			static void* operator new(std::size_t size) { return _pool.allocate(size); }
			static void operator delete(void* ptr, std::size_t size) { return _pool.free(ptr, size); }
		};

		struct Contact
		{
			Body* body1;
			Body* body2;
			Collision collision;
			uint8_t generation;
			uint32_t age;
		};

		// Filter predicates
		typedef std::function<bool(Body*)> predicate;
		static const predicate pred_active;
		static const predicate pred_solid;
		static const predicate pred_sensor;
		static const predicate pred_static;
		static const predicate pred_dynamic;

	private:
		Vector2 world_origin;
		float world_size;
		int world_depth;
		// Used to determine which collisions have been resolved.
		uint8_t generation;

		std::unique_ptr<QuadTree<Body>> tree;
		std::list<std::unique_ptr<Body>> bodies;
		robin_hood::unordered_map<uint32_t, Contact> contacts;

		friend class DebugEffect2;

		// (Re)creates the quad tree.
		void create_tree(void);
		// Collect all entities which are at equal or higher level in the tree as a given body.
		void collect_collisions(const QuadTree<Body>& t, Body* body, std::vector<Body*>& res) const;
		// Tests that a collision between two bodies is valie and if so tracks it.
		void test_collision(Body* this_body, Body* other_body);
		// Attempts to resolve active collisions and notifies at the end of collisions.
		void resolve_collisions(void);

		// Collects ray collisions
		void find(QuadTree<Body>* tree, const Ray2& ray, float min_t, float max_t, std::list<Body*>& list, const predicate& p = pred_active) const;
		Body* find_closest(QuadTree<Body>* tree, const Ray2& ray, float min_t, float max_t, float& t, const predicate& p) const;

		// Generate a hash for a contact between two bodies.
		inline uint32_t hash(const Body* b1, const Body* b2) const { return 65536u * static_cast<uint32_t>(std::min(b1->id, b2->id)) + static_cast<uint32_t>(std::max(b1->id, b2->id)); }

	public:
		CollisionManager2(GameBase* game);
		~CollisionManager2(void) { }

		// Sets the origin of the world (default { 0, 0 }).
		void set_world_origin(const Vector2& world_origin) { this->world_origin = world_origin; create_tree(); }
		// Sets the world size around the origin.
		void set_world_size(float world_size) { this->world_size = world_size; create_tree(); }
		// Sets the depth of the world collision tree. 
		void set_world_depth(int world_depth) { this->world_depth = world_depth; create_tree(); }

		Body* create_body(bool dynamic = true);
		void destroy_body(Body* body);
		void invalidate_contacts(Body* body);
		void update(float delta);

		// Returns the number of collision bodies.
		int body_count(void) const { return static_cast<int>(bodies.size()); }
		// Returns the number of contacts.
		int contact_count(void) const { return static_cast<int>(contacts.size()); }
		// Returns true if there exists a contact between two bodies.
		bool has_contact(Body* b1, Body* b2) const { return contacts.count(hash(b1, b2)) > 0; }
		// Returns true if there exists at least one contact that includes a given body.
		bool has_contact(Body* b) const;
		// Returns all contacts for a given body.
		std::list<Contact*> get_contacts(Body* b) const;

		// Spatial queries

		// Returns all bodies at a given point.
		std::list<Body*> find(const Vector2& pos, const predicate& p = pred_active) const;
		// Returns all bodies which are intersected by a given bounding box.
		std::list<Body*> find(const AABB2& bb, const predicate& p = pred_active) const;
		// Returns all bodies which are intersected by a given ray.
		std::list<Body*> find(const Ray2& ray, float min_t, float max_t, const predicate& p = pred_active) const;
		// Returns closest body intersected by a given ray.
		Body* find_closest(const Ray2& ray, float min_t, float max_t, float& t, const predicate& p = pred_active) const;
	};
}