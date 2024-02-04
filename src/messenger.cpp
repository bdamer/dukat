#include "stdafx.h"
#include <dukat/messenger.h>

namespace dukat
{
	struct EventLock
	{
		Event& active_event;

		EventLock(Event& active_event, const Event& event_trigger) : active_event(active_event)
		{
			active_event = event_trigger;
		}

		~EventLock(void)
		{
			active_event = events::None;
		}
	};

	void Messenger::trigger(const Message& message)
	{
		if (!subscriptions.count(message.event))
			return;
		EventLock lock(active_trigger, message.event);
		const auto& subs = subscriptions.at(message.event);
		for (const auto& r : subs)
			r->receive(message);
	}

	void Messenger::subscribe(Recipient* recipient, Event ev)
	{
		assert(active_trigger != ev);
		if (subscriptions.count(ev) == 0)
			subscriptions.emplace(ev, std::list<Recipient*>());
		auto& list = subscriptions.at(ev);
		auto it = std::find(list.begin(), list.end(), recipient);
		if (it == list.end())
			list.push_back(recipient);
	}

	void Messenger::subscribe(Recipient * recipient, const std::vector<Event>& events)
	{
		std::for_each(events.begin(), events.end(), [&](const Event& e) { subscribe(recipient, e); });
	}

	void Messenger::subscribe_all(Recipient* recipient)
	{
		for (auto it = events::None + 1; it != events::Any; ++it)
			subscribe(recipient, it);
	}

	void Messenger::do_unsubscribe(Recipient* recipient, std::list<Recipient*>& list)
	{
		for (auto it = list.begin(); it != list.end(); ++it)
		{
			if (*it == recipient)
			{
				list.erase(it);
				break;
			}
		}
	}

	void Messenger::unsubscribe(Recipient* recipient, Event ev)
	{
		if (subscriptions.count(ev))
		{
			assert(active_trigger != ev);
			do_unsubscribe(recipient, subscriptions.at(ev));
		}
	}

	void Messenger::unsubscribe(Recipient* recipient, const std::vector<Event>& events)
	{
		std::for_each(events.begin(), events.end(), [&](const Event& e) { unsubscribe(recipient, e); });
	}

	void Messenger::unsubscribe_all(Recipient* recipient)
	{
		for (auto it = subscriptions.begin(); it != subscriptions.end(); ++it)
			do_unsubscribe(recipient, it->second);
	}
}
