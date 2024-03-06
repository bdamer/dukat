#pragma once

namespace dukat
{
    typedef uint32_t Event;
    
    struct Message
    {
        Event event;	// event code
		const void* param1; // parameter 1
		const void* param2; // parameter 2

        Message(void) : event(0), param1(nullptr), param2(nullptr) { }
        Message(Event ev, const void* param1 = nullptr, const void* param2 = nullptr) : event(ev), param1(param1), param2(param2) { }
    
        // convenience methods
        template <typename T>
        const T* get_param1(void) const { return static_cast<const T*>(param1); }
        template <typename T>
        const T* get_param2(void) const { return static_cast<const T*>(param2); }
    };

    // Abstract recipient interface
    class Recipient
    {
    public:
		Recipient(void) { }
		virtual ~Recipient(void) { }

        virtual void receive(const Message& msg) = 0;
    };
}
