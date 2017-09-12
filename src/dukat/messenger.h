#pragma once

#include <unordered_map>
#include <set>
#include "recipient.h"

namespace dukat
{
	struct Events
	{
		static constexpr Event None = 0; // Range marker - does not trigger
		static constexpr Event ToggleDebug = 1;
		static constexpr Event Created = 2;
		static constexpr Event Destroyed = 3;
		static constexpr Event Selected = 4;
		static constexpr Event Deselected = 5;
		static constexpr Event ParentChanged = 6;
		static constexpr Event TransformChanged = 7;
		static constexpr Event VisibilityChanged = 8;
		static constexpr Event Any = 9; // catch-all to allow subscription to all supported events
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
		// Receives a message.
		virtual void receive(const Message& message) = 0;
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