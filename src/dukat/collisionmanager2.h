#pragma once

#include <list>
#include <memory>
#include <unordered_set>

#include "game2.h"
#include "manager.h"
#include "quadtree.h"

namespace dukat
{
	class DebugEffect2;

	class CollisionManager2 : public Manager
	{
	public:	
		struct Body
		{
			const uint16_t id;
			bool dynamic;	// dynamic bodies can be moved as part of collision resolution
			bool solid;		// if true, will cause this body to take part in collision resolution
			bool active;	// if false, will cause this body to be ignored by collision manager
			AABB2 bb;
			Messenger* owner;

			Body(uint16_t id) : id(id), dynamic(true), solid(true), active(true), owner(nullptr) { }
		};

		struct Contact
		{
			Body* body1;
			Body* body2;
			Collision collision;
			bool active;
		};

	private:
		Vector2 world_origin;
		float world_size;
		int world_depth;

		std::unique_ptr<QuadTree<Body>> tree;
		std::list<std::unique_ptr<Body>> bodies;
		std::unordered_map<uint32_t, Contact> contacts;

		friend class DebugEffect2;

		// (Re)creates the quad tree.
		void create_tree(void);
		// Collect all entities which are at equal or higher level in the tree as a given body.
		void find_collisions(const QuadTree<Body>& t, Body* body, std::vector<Body*>& res) const;
		// Generate a hash for a contact between two bodies.
		inline uint32_t hash(Body* b1, Body* b2) const { return 65536u * static_cast<uint32_t>(std::min(b1->id, b2->id)) + static_cast<uint32_t>(std::max(b1->id, b2->id)); }

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

		// Returns the number of collision bodies.
		int body_count(void) const { return static_cast<int>(bodies.size()); }
		// Returns the number of contacts.
		int contact_count(void) const { return static_cast<int>(contacts.size()); }
		// Returns true if there exists a contact between two bodies.
		bool has_contact(Body* b1, Body* b2) const { return contacts.count(hash(b1, b2)) > 0; }
		// Returns all contacts for a given body.
		std::list<Contact*> get_contacts(Body* b) const;
		// Returns all bodies at point p.
		std::list<Body*> get_bodies(const Vector2& p) const;

		void update(float delta);
	};
}