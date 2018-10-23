#include "stdafx.h"
#include <dukat/timermanager.h>

namespace dukat
{
    Timer* TimerManager::create_timer(float interval, std::function<void(void)> callback, bool recurring)
    {
        auto timer = std::make_unique<Timer>(++last_id, interval, callback, recurring);
        auto res = timer.get();
        timers.push_back(std::move(timer));
		return res;
    }
    
    void TimerManager::cancel_timer(Timer* timer)
    {
        auto it = std::find_if(timers.begin(), timers.end(), [timer](const std::unique_ptr<Timer>& ptr) {
            return ptr.get() == timer;
        });
        if (it != timers.end())
        {
            timers.erase(it);
        }
    }

    void TimerManager::update(float delta)
    {
        for (auto it = timers.begin(); it != timers.end(); )
        {
            (*it)->remaining -= delta;
            // check if timer has expired
            if ((*it)->remaining <= 0.0f)
            {
                if ((*it)->callback)
                {
                    (*it)->callback();
                }

                if ((*it)->recurring)
                {
                    (*it)->remaining = (*it)->interval;
                }
                else
                {
                    it = timers.erase(it);
                    continue;
                }
            }

            ++it;
        }
    }
}