#pragma once

#include <functional>
#include <list>
#include <memory>

#include "manager.h"
#include "objectpool.h"

namespace dukat
{
    struct Timer
    {
		uint32_t generation;
		uint8_t group; // timer group
		float interval;
		float runtime; 
        bool recurring;
		bool alive;
        std::function<void(void)> callback;

        Timer(void) : generation(0u), group(0u), interval(0.0f), runtime(0.0f), 
			recurring(false), alive(false), callback(nullptr) { }
    };

    class TimerManager : public Manager
    {
    private:
		// max number of timers
		static constexpr auto max_timers = 1024;
		uint32_t generation;
		uint8_t active_group;
		ObjectPool<Timer, max_timers> timers;

    public:
		TimerManager(GameBase* game) : Manager(game), generation(0u), active_group(0u) { }
		~TimerManager(void) { }

		Timer* create(float interval, std::function<void(void)> callback, bool recurring = false);
		void cancel(Timer* timer) { if (timer != nullptr) timers.release(timer); }
		void update(float delta);
		void set_active_group(uint8_t group) { this->active_group = group; }

		// Deprecated - use create instead
		Timer* create_timer(float interval, std::function<void(void)> callback, bool recurring = false) { return create(interval, callback, recurring); }
		// Deprecated - use cancel instead
		void cancel_timer(Timer* timer) { cancel(timer); }

		// Cancels all active timers.
		void clear(void) { timers.clear(); }
    };
}