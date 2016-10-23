#pragma once

#include <functional>
#include <list>
#include <memory>

namespace dukat
{
    struct Timer
    {
        float interval;
        float remaining;
        bool recurring;
        std::function<void(void)> callback;

        Timer(void) : interval(0.0f), remaining(0.0f), recurring(false) { }
        Timer(float interval, std::function<void(void)> callback, bool recurring = false) : interval(interval), remaining(interval), callback(callback), recurring(recurring) { }
    };

    class TimerManager
    {
    private:
        std::list<std::unique_ptr<Timer>> timers;

    public:
        Timer* create_timer(float interval, std::function<void(void)> callback, bool recurring = false);
        void cancel_timer(Timer* timer);
        void update(float delta);
    };
}