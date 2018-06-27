#pragma once

#include <unordered_map>
#include <set>
#include "recipient.h"

namespace dukat
{
	struct Events
	{
		// Range marker - does not trigger
		static constexpr Event None = 0;
		// System events
		static constexpr Event ToggleDebug = 1;
		static constexpr Event WindowResized = 2;
		// Entity Events
		static constexpr Event Created = 10;
		static constexpr Event Destroyed = 11;
		static constexpr Event Selected = 12;
		static constexpr Event Deselected = 13;
		static constexpr Event ParentChanged = 14;
		static constexpr Event TransformChanged = 15;
		static constexpr Event VisibilityChanged = 16;
		// Marks begin of a collision.
		// param1: Body* that entity collided with.
		// param2: Contact* contact of this collision.
		static constexpr Event CollisionBegin = 17;
		// Marks end of a collision.
		// param1: Body* that entity collided with.
		static constexpr Event CollisionEnd = 18;
		// Indicates that a collision was resolved.
		// param1: Vector2* direction of resolution.
		static constexpr Event CollisionResolve = 19; 
		// catch-all to allow subscription to all supported events
		// TODO: review - I don't like the hard-coded max ID here
		static constexpr Event Any = 64;
	};

	// Messenging class
	class Messenger
	{
	private:
		// Map of subscribers, indexed by event type
		std::unordered_map<Event, std::set<Recipient*>> subscriptions;

	public:
		Messenger(void) { }
		virtual ~Messenger(void) { }

		// Triggers an event for all recievers subscribed to this entity.
		void trigger(const Message& message);
		// Subscribes to an event on this entity.
		void subscribe(Event ev, Recipient* recipient);
		// Subscribes to all events on this entity.
		void subscribe_all(Recipient* recipient);
		// Unsubscribes from an event on this entity.
		void unsubscribe(Event ev, Recipient* recipient);
		// Unsubscribes from all events this recipient was registered for.
		void unsubscribe_all(Recipient* recipient);
	};
}