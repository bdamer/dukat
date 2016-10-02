#pragma once

#include <set>
#include "recipient.h"
#include "event.h"

namespace dukat
{
	// Messenging class
	class Messenger
	{
	private:
		// Map of subscribers, indexed by event type
		std::map<int, std::set<Recipient*>> subscriptions;

	public:
		Messenger(void) { }
		virtual ~Messenger(void) { }

		// Triggers an event for all recievers subscribed to this entity.
		void trigger(const Message& message);
		// Receives a message.
		virtual void receive(const Message& message) = 0;
		// Subscribes to an event on this entity.
		void subscribe(Event ev, Recipient* recipient);
		void subscribe_all(Recipient* recipient);
		// Unsubscribes from an event on this entity.
		void unsubscribe(Event ev, Recipient* recipient);
		void unsubscribe_all(Recipient* recipient);
	};
}