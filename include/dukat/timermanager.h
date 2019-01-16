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
		uint32_t generation;
		uint8_t group; // timer group
		float interval;
		float runtime; 
        bool recurring;
		bool alive;
        std::function<void(void)> callback;

        Timer(void) : id(0u), generation(0u), group(0u), interval(0.0f), runtime(0.0f), 
			recurring(false), alive(false), callback(nullptr) { }
    };

    class TimerManager : public Manager
    {
    private:
		// increments at which to allocate timers
		static constexpr auto allocation_count = 512;

		uint32_t generation;
		uint8_t active_group;
		std::vector<std::unique_ptr<Timer>> timers;

		void allocate_timers(void);

    public:
		TimerManager(GameBase* game) : Manager(game), generation(0u), active_group(0u) { allocate_timers(); }
		~TimerManager(void) { }

        Timer* create_timer(float interval, std::function<void(void)> callback, bool recurring = false);
        void cancel_timer(Timer* timer);
        void update(float delta);
		void set_active_group(uint8_t group) { this->active_group = group; }

		// Cancels all active timers.
		void clear(void) { timers.clear(); }
    };
}