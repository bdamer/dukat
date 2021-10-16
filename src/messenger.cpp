#include "stdafx.h"
#include <dukat/messenger.h>

namespace dukat
{
	void Messenger::trigger(const Message& message)
	{
		if (!subscriptions.count(message.event))
			return;
		active_trigger = message.event;
		const auto& subs = subscriptions.at(message.event);
		for (const auto& r : subs)
			r->receive(message);
		active_trigger = Events::None;
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
		for (auto it = Events::None + 1; it != Events::Any; ++it)
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
