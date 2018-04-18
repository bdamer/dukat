#pragma once

#ifdef BOX2D_SUPPORT

#include <Box2D/Box2D.h>
#include <queue>

#include "manager.h"
#include "messenger.h"

namespace dukat
{
	// Box2D collision manager implementation.
	class Box2DManager : public Manager, public b2ContactListener, public Messenger
	{
	private:
		// Box2D world for physics simulation
		std::unique_ptr<b2World> box_world;
		std::queue<Message> messages;

	public:
		Box2DManager(GameBase* game);
		~Box2DManager(void);

		void update(float delta);

		b2Body* create_body(const b2BodyDef* definition) { return box_world->CreateBody(definition); }
		void destroy_body(b2Body* body) { box_world->DestroyBody(body); }
		b2World* get_world(void) { return box_world.get(); }

		// Box2D World Callbacks
		void BeginContact(b2Contact* contact);
		void EndContact(b2Contact* contact);
	};
}

#endif