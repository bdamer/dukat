#include "stdafx.h"
#include <dukat/timermanager.h>
#include <dukat/perfcounter.h>

namespace dukat
{
	MemoryPool<Timer> Timer::_pool(256);

	Timer* TimerManager::create(float interval, std::function<void(void)> callback, bool recurring)
    {
		assert(interval >= 0);

		// initialize timer
		auto t = std::make_unique<Timer>();
		t->deferred = std::make_unique<Deferred>();
		if (callback != nullptr)
			t->deferred->then(callback);
		t->generation = generation;
		t->group = active_group;
		t->interval = interval;
		t->recurring = recurring;
		t->runtime = 0.0f;

		auto res = t.get();
		timers.push_back(std::move(t));
		return res;
    }

	void TimerManager::cancel(Timer* timer)
	{
		if (timer != nullptr) 
			timer->alive = false;
	}
    
    void TimerManager::update(float delta)
    {
		generation++;

		for (auto it = timers.begin(); it != timers.end(); )
		{
			// clear dead timers
			if (!(*it)->alive)
			{
				it = timers.erase(it);
				continue;
			}

			auto& t = *(*it);
			// only process timers that have been created before this frame
			// only process timers of group 0 or active group
			if (t.generation != generation && (t.group == active_group || t.group == 0))
			{
				t.runtime += delta;
				// check if timer has expired
				if (t.runtime >= t.interval)
				{
					t.deferred->resolve();

					if (t.recurring) // reset timer, accounting for any time over interval
						t.runtime = t.runtime - t.interval;
					else
						t.alive = false;
				}
			}

			++it;
        }
		perfc.inc(PerformanceCounter::TIMERS, timers.size());
    }
}