#pragma once

namespace dukat
{
    typedef uint32_t Event;
    
    struct Message
    {
        Event event;	// event code
		const void* param1; // parameter 1
		const void* param2; // parameter 2

        Message(void) { }
        Message(Event ev, const void* param1 = nullptr, const void* param2 = nullptr) : event(ev), param1(param1), param2(param2) { }
    };

    // Abstract recipient interface
    class Recipient
    {
    public:
        virtual void receive(const Message& msg) = 0;
    };
}
