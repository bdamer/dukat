#include "stdafx.h"
#include <dukat/box2dmanager.h>
#include <dukat/game2.h>
#include <dukat/recipient.h>

#ifdef BOX2D_SUPPORT

namespace dukat
{
	Box2DManager::Box2DManager(GameBase* game) : Manager(game)
	{
		box_world = std::make_unique<b2World>(b2Vec2_zero);
		box_world->SetContactListener(this);
	}

	Box2DManager::~Box2DManager(void)
	{
		box_world->SetContactListener(nullptr);
	}

	void Box2DManager::update(float delta)
	{
		box_world->Step(delta, 6, 2);
		while (!messages.empty())
		{
			auto& msg = messages.front();
			trigger(msg);
			messages.pop();
		}
	}

	void Box2DManager::BeginContact(b2Contact* contact)
	{
		// just queue message for now; we won't actually send it until we've
		// left the b2 simulation step.
		Message msg(events::CollisionBegin);
		msg.param1 = static_cast<const void*>(contact->GetFixtureA());
		msg.param2 = static_cast<const void*>(contact->GetFixtureB());
		messages.push(msg);
	}

	void Box2DManager::EndContact(b2Contact* contact)
	{
		// This can be called either during b2 step or when destroying a body, so
		// send notification immediately
		Message msg(events::CollisionEnd);
		msg.param1 = static_cast<const void*>(contact->GetFixtureA());
		msg.param2 = static_cast<const void*>(contact->GetFixtureB());
		trigger(msg);
	}
}

#endif