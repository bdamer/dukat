#include "stdafx.h"
#include <dukat/inputdevice.h>
#include <dukat/settings.h>

namespace dukat
{
	std::array<std::function<void(void)>, InputDevice::VirtualButton::_Count> InputDevice::button_handlers;
	std::array<std::function<void(void)>, InputDevice::VirtualButton::_Count> InputDevice::long_press_handlers;

	InputDevice::InputDevice(const Window& window, const Settings& settings, bool digital) : window(window), lx(0.0f), ly(0.0f),
		rx(0.0f), ry(0.0f), lxa(0.0f), lya(0.0f), rxa(0.0f), rya(0.0f), lt(0.0f), rt(0.0f), digital(digital)
	{
		std::fill(buttons.begin(), buttons.end(), 0x0);
		long_press_threshold = settings.get_int("input.longpress", 1000);
	}

	void InputDevice::update_button_state(VirtualButton button, bool pressed)
	{
		const auto was_pressed = buttons[button] > 0;
		if (pressed != was_pressed) // state was changed
		{
			buttons[button] = pressed ? SDL_GetTicks() : 0;
			if (pressed && button_handlers[button] != nullptr)
				button_handlers[button]();
		}

		// Check if long press has been reached
		if (pressed && long_press_handlers[button] != nullptr && buttons[button] != -1 &&
			(SDL_GetTicks() - buttons[button]) >= long_press_threshold)
		{
			long_press_handlers[button]();
			buttons[button] = static_cast<Uint32>(-1); // prevent long press from firing again
		}
	}
}