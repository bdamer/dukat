#include "stdafx.h"
#include <dukat/timermanager.h>
#include <dukat/perfcounter.h>

namespace dukat
{
	Timer* TimerManager::create(float interval, std::function<void(void)> callback, bool recurring)
    {
		assert(interval >= 0);

		// Find unused timer
		Timer* t = timers.acquire();
		if (t == nullptr) // no timers available
			return nullptr; 

		// initialize timer
		t->callback = callback;
		t->generation = generation;
		t->group = active_group;
		t->interval = interval;
		t->recurring = recurring;
		t->runtime = 0.0f;
		return t;
    }
    
    void TimerManager::update(float delta)
    {
		for (auto it : free_list)
			timers.release(*it);
		free_list.clear();

		generation++;

		auto alive = 0;
		timers.free_index = 0;
		for (auto& t : timers.data)
		{
			// only process live timers
			if (!t.alive)
				continue;
			// only process timers that have been created before this frame
			if (t.generation == generation)
				continue;
			// only process timers of group 0 or active group
			if (t.group != active_group && t.group > 0)
				continue;
			alive++;
			t.runtime += delta;
            // check if timer has expired
            if (t.runtime >= t.interval)
            {
                if (t.callback)
                    t.callback();

				if (t.recurring) // reset timer, accounting for any time over interval
					t.runtime = t.runtime - t.interval;
				else
					free_list.insert(&t);
            }
        }
		perfc.inc(PerformanceCounter::TIMERS, alive);
    }
}