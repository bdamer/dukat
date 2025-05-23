#include "stdafx.h"
#include <dukat/inputdevice.h>
#include <dukat/log.h>
#include <dukat/settings.h>

namespace dukat
{
	const std::string InputDevice::default_profile = "default";
	std::array<std::function<void(void)>, InputDevice::VirtualButton::_Count> InputDevice::button_handlers;
	std::array<LongPressHandler, InputDevice::VirtualButton::_Count> InputDevice::long_press_handlers;

	InputDevice::InputDevice(const Window& window, const Settings& settings, bool digital) : window(window), lx(0.0f), ly(0.0f),
		rx(0.0f), ry(0.0f), lxa(0.0f), lya(0.0f), rxa(0.0f), rya(0.0f), lt(0.0f), rt(0.0f), digital(digital)
	{
		std::fill(buttons.begin(), buttons.end(), 0x0);
		long_press_threshold = settings.get_int(settings::input_longpress, 1000);
	}

	void InputDevice::on_press(VirtualButton button, std::function<void(void)> handler)
	{ 
		log->trace("Binding button: {}", static_cast<int>(button));
		button_handlers[button] = handler; 
	}

	void InputDevice::unbind(VirtualButton button) 
	{ 
		log->trace("Unbinding button: {}", static_cast<int>(button));
		button_handlers[button] = nullptr;
	}

	void InputDevice::bind_long_press(VirtualButton button, const std::function<void(LongPressHandler::Event)>& handler, Uint32 threshold)
	{
		long_press_handlers[button] = LongPressHandler(threshold > 0 ? threshold : long_press_threshold, handler);
	}

	bool InputDevice::is_mapped(int key, VirtualButton& button) const
	{
		for (auto i = 0u; i < mapping.size(); i++)
		{
			if (key == mapping[i])
			{
				button = static_cast<VirtualButton>(i);
				return true;
			}
		}
		return false;
	}

	void InputDevice::update_button_state(VirtualButton button, bool pressed)
	{
		const auto was_pressed = buttons[button] > 0;
		const auto& lp_handler = long_press_handlers[button];

		if (pressed != was_pressed) // state was changed
		{
			buttons[button] = pressed ? SDL_GetTicks() : 0;
			if (pressed && button_handlers[button] != nullptr)
				button_handlers[button]();

			if (lp_handler != nullptr)
				lp_handler.callback(pressed ? LongPressHandler::Begin : LongPressHandler::Cancel);
		}

		// Check if long press has been reached
		if (pressed && lp_handler != nullptr && buttons[button] != -1 &&
			(SDL_GetTicks() - buttons[button]) >= lp_handler.threshold)
		{
			lp_handler.callback(LongPressHandler::Complete);
			buttons[button] = static_cast<Uint32>(-1); // prevent long press from firing again
		}
	}
}