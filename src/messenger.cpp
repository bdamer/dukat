#include "stdafx.h"
#include <dukat/messenger.h>

namespace dukat
{
	void Messenger::trigger(const Message& message)
	{
		if (subscriptions.count(message.event))
		{
			for (auto r : subscriptions[message.event])
			{
				r->receive(message);
			}
		}
	}

	void Messenger::subscribe(Recipient * recipient, Event ev)
	{
		if (subscriptions.count(ev) == 0)
		{
			subscriptions[ev] = std::set<Recipient*>();
		}
		subscriptions[ev].insert(recipient);
	}

	void Messenger::subscribe(Recipient * recipient, const std::vector<Event>& events)
	{
		std::for_each(events.begin(), events.end(), [&](const Event& e) { subscribe(recipient, e); });
	}

	void Messenger::subscribe_all(Recipient* recipient)
	{
		for (auto it = Events::None; it != Events::Any; ++it)
		{
			subscribe(recipient, it);
		}
	}

	void Messenger::unsubscribe(Recipient* recipient, Event ev)
	{
		if (subscriptions.count(ev))
		{
			subscriptions[ev].erase(recipient);
		}
	}

	void Messenger::unsubscribe(Recipient* recipient, const std::vector<Event>& events)
	{
		std::for_each(events.begin(), events.end(), [&](const Event& e) { unsubscribe(recipient, e); });
	}

	void Messenger::unsubscribe_all(Recipient* recipient)
	{
		for (auto it = subscriptions.begin(); it != subscriptions.end(); ++it)
		{
			it->second.erase(recipient);
		}
	}
}
