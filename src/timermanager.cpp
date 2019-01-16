#include "stdafx.h"
#include <dukat/timermanager.h>

namespace dukat
{
	void TimerManager::allocate_timers(void)
	{
		auto old_size = timers.size();
		auto new_size = old_size + allocation_count;
		timers.resize(new_size);
		for (auto i = old_size; i < new_size; i++)
		{
			auto timer = std::make_unique<Timer>();
			timer->id = i;
			timer->generation = generation;
			timers[i] = std::move(timer);
		}
	}

	Timer* TimerManager::create_timer(float interval, std::function<void(void)> callback, bool recurring)
    {
		// Find unused timer
		Timer* t = nullptr;
		for (auto it = timers.begin(); it != timers.end(); ++it)
		{
			if (!(*it)->alive)
			{
				t = (*it).get();
				break;
			}
		}

		// no timers available, so allocate more
		if (t == nullptr)
		{
			auto offset = timers.size();
			allocate_timers();
			t = timers[offset].get();
		}

		// initialize timer
		t->alive = true;
		t->callback = callback;
		t->generation = generation;
		t->group = active_group;
		t->interval = interval;
		t->recurring = recurring;
		t->runtime = 0.0f;
		return t;
    }
    
    void TimerManager::cancel_timer(Timer* timer)
    {
		if (timer != nullptr)
		{
			timer->alive = false;
		}
    }

    void TimerManager::update(float delta)
    {
		generation++;

		for (auto it = timers.begin(); it != timers.end(); ++it)
		{
			auto& t = (*it);
			// only process live timers
			if (!t->alive)
				continue;
			// only process timers that have been created before this frame
			if (t->generation == generation)
				continue;
			// only process timers of group 0 or active group
			if (t->group != active_group && t->group > 0)
				continue;

            t->runtime += delta;
            // check if timer has expired
            if (t->runtime >= t->interval)
            {
                if (t->callback)
                {
                    t->callback();
                }

                if (t->recurring)
                {
					// reset timer, accounting for any time over interval
                    t->runtime = t->runtime - t->interval;
                }
                else
                {
					t->alive = false;
                }
            }
        }
    }
}