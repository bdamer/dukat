#pragma once

#include <functional>
#include <list>
#include <memory>

#include "manager.h"
#include "memorypool.h"

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
			recurring(false), alive(true), callback(nullptr) { }

		// Custom memory allocation
		static MemoryPool<Timer> _pool;
		static void* operator new(std::size_t size) { return _pool.allocate(size); }
		static void operator delete(void* ptr, std::size_t size) { return _pool.free(ptr, size); }
    };

    class TimerManager : public Manager
    {
    private:
		uint32_t generation;
		uint8_t active_group;
		std::list<std::unique_ptr<Timer>> timers;

    public:
		TimerManager(GameBase* game) : Manager(game), generation(0u), active_group(0u) { }
		~TimerManager(void) { }

		Timer* create(float interval, std::function<void(void)> callback, bool recurring = false);
		void cancel(Timer* timer) { if (timer != nullptr) timer->alive = false; }
		void update(float delta);
		void set_active_group(uint8_t group) { this->active_group = group; }

		// Cancels all active timers.
		void clear(void) { timers.clear(); }
    };
}