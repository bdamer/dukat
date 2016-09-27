#pragma once

namespace dukat
{
    struct Message
    {
        int event;	// event code
        void* param1; // parameter 1
        void* param2; // parameter 2

        Message(void) { }
        Message(int ev, void* param1 = nullptr, void* param2 = nullptr) : event(ev), param1(param1), param2(param2) { }
    };

    // Abstract recipient interface
    class Recipient
    {
    public:
        virtual void receive(const Message& msg) = 0;
    };
}
