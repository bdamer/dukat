#pragma once

#include <functional>
#include <list>

namespace dukat
{
	struct Deferred
	{
		std::list<std::function<void(void)>> callbacks;
		void resolve(void) { for (auto& c : callbacks) c(); }
		void then(std::function<void(void)> callback) { callbacks.push_back(callback); }
	};
}