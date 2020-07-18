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
		// Notification that controller has been added.
		static constexpr Event DeviceBound = 3;
		// Notification that controller has been removed.
		static constexpr Event DeviceUnbound = 4;
		// Game events
		// Game is about to start.
		// param1: bool* true if new game, false if existing game was restored.
		static constexpr Event GameBegin = 5;
		// Game is about to end
		static constexpr Event GameEnd = 6;
		// Sent after camera was changed
		static constexpr Event CameraChanged = 9;
		// Entity Events
		// Entity has been created.
		// param1: Entity* The entity that has been created.
		static constexpr Event Created = 10;
		// Entity is about to be destroyed
		// param1: Entity* The entity that will be destroyed.
		static constexpr Event Destroyed = 11;
		static constexpr Event Activated = 12;
		static constexpr Event Deactivated = 13;
		static constexpr Event Selected = 14;
		static constexpr Event Deselected = 15;
		static constexpr Event ParentChanged = 16;
		static constexpr Event TransformChanged = 17;
		static constexpr Event VisibilityChanged = 18;
		static constexpr Event LayerChanged = 19;
		// Marks begin of a collision.
		// param1: Body* that entity collided with.
		// param2: Contact* contact of this collision.
		static constexpr Event CollisionBegin = 20;
		// Marks end of a collision.
		// param1: Body* that entity collided with.
		static constexpr Event CollisionEnd = 21;
		// Indicates that a collision was resolved.
		// param1: Vector2* direction of resolution.
		static constexpr Event CollisionResolve = 22; 
		// Indicates that a collision body was created.
		// param1: Body* collision body
		static constexpr Event BodyCreated = 23;
		// Indicates that a collision body was destroyed.
		// param1: Body* collision body
		static constexpr Event BodyDestroyed = 24;
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
		void subscribe(Recipient* recipient, Event ev);
		void subscribe(Recipient* recipient, const std::vector<Event>& events);
		void subscribe(Event ev, Recipient* recipient) { subscribe(recipient, ev); } // legacy
		// Subscribes to all events on this entity.
		void subscribe_all(Recipient* recipient);
		// Unsubscribes from an event on this entity.
		void unsubscribe(Recipient* recipient, Event ev);
		void unsubscribe(Recipient* recipient, const std::vector<Event>& events);
		void unsubscribe(Event ev, Recipient* recipient) { unsubscribe(recipient, ev); } // legacy
		// Unsubscribes from all events this recipient was registered for.
		void unsubscribe_all(Recipient* recipient);
	};
}