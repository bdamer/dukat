#include "stdafx.h"
#include "inputdevice.h"

namespace dukat
{
	std::map<InputDevice::VirtualButton, std::function<void(void)>> InputDevice::handlers;

	void InputDevice::update_button_state(VirtualButton button, bool pressed)
	{
		// state was changed
		if (pressed != buttons[button])
		{
			buttons[button] = pressed;
			if (pressed && handlers.count(button) > 0)
			{
				handlers[button]();
			}
		}
	}

}