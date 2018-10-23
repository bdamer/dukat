#pragma once

#include "recipient.h"

namespace Json
{
	class Value;
}

namespace dukat
{
	// Forward declarations
	class Entity;

	// Component base class
	class Component : public Recipient
	{
	public:
		Entity* entity;

		Component(Entity* entity) : entity(entity) { }
		virtual ~Component(void) { }

		// Updates the component state.
		virtual void update(float delta) { }
        // Receives a message.
        virtual void receive(const Message& message) { }
	
		// Serialization
		virtual void serialize(Json::Value& node) { }
		virtual void deserialize(const Json::Value& node) { }
	};
}