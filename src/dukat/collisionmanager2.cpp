#include "stdafx.h"
#include "collisionmanager2.h"
#include "game2.h"
#include "recipient.h"

namespace dukat
{
	CollisionManager2::CollisionManager2(Game2* game) : game(game)
	{
		box_world = std::make_unique<b2World>(b2Vec2_zero);
		box_world->SetContactListener(this);
	}

	CollisionManager2::~CollisionManager2(void)
	{
		box_world->SetContactListener(nullptr);
	}

	void CollisionManager2::update(float delta)
	{
		box_world->Step(delta, 6, 2);
		while (!messages.empty())
		{
			auto& msg = messages.front();
			trigger(msg);
			messages.pop();
		}
	}

	void CollisionManager2::BeginContact(b2Contact* contact)
	{
		// just queue message for now; we won't actually send it until we've
		// left the b2 simulation step.
		Message msg(Events::CollisionBegin);
		msg.param1 = static_cast<const void*>(contact->GetFixtureA());
		msg.param2 = static_cast<const void*>(contact->GetFixtureB());
		messages.push(msg);
	}

	void CollisionManager2::EndContact(b2Contact* contact)
	{
		// This can be called either during b2 step or when destroying a body, so
		// send notification immediately
		Message msg(Events::CollisionEnd);
		msg.param1 = static_cast<const void*>(contact->GetFixtureA());
		msg.param2 = static_cast<const void*>(contact->GetFixtureB());
		trigger(msg);
	}
}