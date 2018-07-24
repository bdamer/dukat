#pragma once

#include <functional>
#include <list>
#include <memory>

#include "manager.h"

namespace dukat
{
    struct Timer
    {
		uint32_t id;
        float interval;
        float remaining;
        bool recurring;
        std::function<void(void)> callback;

        Timer(void) : id(0u), interval(0.0f), remaining(0.0f), recurring(false) { }
        Timer(uint32_t id, float interval, std::function<void(void)> callback, bool recurring = false)
			: id(id), interval(interval), remaining(interval), callback(callback), recurring(recurring) { }
    };

    class TimerManager : public Manager
    {
    private:
		uint32_t last_id;
        std::list<std::unique_ptr<Timer>> timers;

    public:
		TimerManager(GameBase* game) : Manager(game), last_id(0u) { }
		~TimerManager(void) { }

        Timer* create_timer(float interval, std::function<void(void)> callback, bool recurring = false);
        void cancel_timer(Timer* timer);
        void update(float delta);

		// Cancels all active timers.
		void clear(void) { timers.clear(); }
    };
}