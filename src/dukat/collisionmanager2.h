#pragma once

#include <Box2D/Box2D.h>
#include <queue>
#include "messenger.h"

namespace dukat
{
	class Game2;

	// Box2D collision manager implementation.
	class CollisionManager2 : public b2ContactListener, public Messenger
	{
	private:
		Game2* game;
		// Box2D world for physics simulation
		std::unique_ptr<b2World> box_world;
		std::queue<Message> messages;

	public:
		CollisionManager2(Game2* game);
		~CollisionManager2(void);

		void update(float delta);

		b2Body* create_body(const b2BodyDef* definition) { return box_world->CreateBody(definition); }
		void destroy_body(b2Body* body) { box_world->DestroyBody(body); }
		b2World* get_world(void) { return box_world.get(); }

		// Box2D World Callbacks
		void BeginContact(b2Contact* contact);
		void EndContact(b2Contact* contact);
	};
}
