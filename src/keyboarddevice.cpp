#include "stdafx.h"
#include <dukat/keyboarddevice.h>
#include <dukat/window.h>
#include <dukat/mathutil.h>
#include <dukat/sysutil.h>
#include <dukat/settings.h>

namespace dukat
{
	KeyboardDevice::KeyboardDevice(const Window& window, const Settings& settings) 
		: InputDevice(window, settings, true), mouse_buttons({ 0, 0, 0 })
	{
		name = "Keyboard";
		sensitivity = settings.get_int("input.mouse.sensitivity", 128);
		restore_mapping(settings);
		// Initialize default key bindings
		keystate = SDL_GetKeyboardState(&num_keys);
	}

	KeyboardDevice::~KeyboardDevice(void)
	{
		SDL_SetRelativeMouseMode(SDL_FALSE);
		keystate = nullptr; // memory handled by SDL
	}

	int KeyboardDevice::get_mapping(VirtualButton button) const
	{
		for (auto i = 0; i < static_cast<int>(mouse_mapping.size()); i++)
		{
			if (mouse_mapping[i] == static_cast<int>(button))
				return mouse_button_to_key(i + 1); // map to [-2,-6]
		}
		return InputDevice::get_mapping(button);
	}

	int KeyboardDevice::get_mouse_mapping(VirtualButton button) const
	{
		for (auto i = 0; i < static_cast<int>(mouse_mapping.size()); i++)
		{
			if (mouse_mapping[i] == static_cast<int>(button))
				return mouse_button_to_key(i + 1); // map to [-2,-6]
		}
		return no_key;
	}

	int KeyboardDevice::get_key_mapping(VirtualButton button) const
	{
		return InputDevice::get_mapping(button);
	}

	bool KeyboardDevice::is_mapped(int key, VirtualButton& button) const
	{
		if (key < InputDevice::no_key) // indicates mouse button
		{
			const auto mouse_button = -key - 2;
			if (mouse_button < static_cast<int>(mouse_mapping.size()) && mouse_mapping[mouse_button] != no_key)
			{
				button = static_cast<VirtualButton>(mouse_mapping[mouse_button]);
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return InputDevice::is_mapped(key, button);
		}
	}

	void KeyboardDevice::override_mapping(VirtualButton button, int key)
	{
		if (key < InputDevice::no_key) // indicates mouse button [1..5]
		{
			const auto mouse_button = -key - 2;
			mouse_mapping[mouse_button] = button;
		}
		else
		{
			InputDevice::override_mapping(button, key);
		}
	}

	void KeyboardDevice::clear_mapping(VirtualButton button)
	{
		for (auto& it : mouse_mapping)
		{
			if (it == button)
				it = no_key;
		}
		InputDevice::clear_mapping(button);
	}

	void KeyboardDevice::restore_mapping(const Settings& settings)
	{
		// Initialize key mapping
		mapping[VirtualButton::Button1] = settings.get_int("input.keyboard.button1", no_key);
		mapping[VirtualButton::Button2] = settings.get_int("input.keyboard.button2", no_key);
		mapping[VirtualButton::Button3] = settings.get_int("input.keyboard.button3", no_key);
		mapping[VirtualButton::Button4] = settings.get_int("input.keyboard.button4", no_key);
		mapping[VirtualButton::Button5] = settings.get_int("input.keyboard.button5", no_key);
		mapping[VirtualButton::Button6] = settings.get_int("input.keyboard.button6", no_key);
		mapping[VirtualButton::Button7] = settings.get_int("input.keyboard.button7", no_key);
		mapping[VirtualButton::Button8] = settings.get_int("input.keyboard.button8", no_key);
		mapping[VirtualButton::Select] = SDL_SCANCODE_ESCAPE;
		mapping[VirtualButton::Start] = SDL_SCANCODE_RETURN;
		mapping[VirtualButton::Down] = settings.get_int("input.keyboard.down", SDL_SCANCODE_DOWN);
		mapping[VirtualButton::Right] = settings.get_int("input.keyboard.right", SDL_SCANCODE_RIGHT);
		mapping[VirtualButton::Left] = settings.get_int("input.keyboard.left", SDL_SCANCODE_LEFT);
		mapping[VirtualButton::Up] = settings.get_int("input.keyboard.up", SDL_SCANCODE_UP);
		mapping[VirtualButton::LeftTrigger] = settings.get_int("input.keyboard.lt", SDL_SCANCODE_TAB);
		mapping[VirtualButton::RightTrigger] = settings.get_int("input.keyboard.rt", SDL_SCANCODE_SPACE);
		mapping[VirtualButton::LeftAxisDown] = settings.get_int("input.keyboard.leftaxis.down", SDL_SCANCODE_S);
		mapping[VirtualButton::LeftAxisRight] = settings.get_int("input.keyboard.leftaxis.right", SDL_SCANCODE_D);
		mapping[VirtualButton::LeftAxisLeft] = settings.get_int("input.keyboard.leftaxis.left", SDL_SCANCODE_A);
		mapping[VirtualButton::LeftAxisUp] = settings.get_int("input.keyboard.leftaxis.up", SDL_SCANCODE_W);
		mapping[VirtualButton::Debug] = settings.get_int("input.keyboard.debug", SDL_SCANCODE_F1);
		// Initialize mouse mapping
		mouse_mapping[0] = settings.get_int("input.mouse.left", VirtualButton::Button1);
		mouse_mapping[1] = settings.get_int("input.mouse.middle", VirtualButton::Button3);
		mouse_mapping[2] = settings.get_int("input.mouse.right", VirtualButton::Button2);
		mouse_mapping[3] = settings.get_int("input.mouse.extra1", VirtualButton::Button4);
		mouse_mapping[4] = settings.get_int("input.mouse.extra2", VirtualButton::Button5);
	}

	void KeyboardDevice::update(void)
	{
		SDL_PumpEvents();

		// Handle direction keys
		const auto key_left = mapping[VirtualButton::LeftAxisLeft];
		const auto key_right = mapping[VirtualButton::LeftAxisRight];
		if (key_left > no_key && key_right > no_key)
		{
			lx = keystate[key_left] ? -1.0f : (keystate[key_right] ? 1.0f : 0.0f);
		}
		const auto key_up = mapping[VirtualButton::LeftAxisUp];
		const auto key_down = mapping[VirtualButton::LeftAxisDown];
		if (key_up > no_key && key_down > no_key)
		{
			ly = keystate[key_up] ? 1.0f : (keystate[key_down] ? -1.0f : 0.0f);
		}

		// Handle trigger keys
		const auto key_lt = mapping[VirtualButton::LeftTrigger];
		lt = key_lt > no_key && keystate[key_lt] ? 1.0f : 0.0f;
		const auto key_rt = mapping[VirtualButton::RightTrigger];
		rt = key_rt > no_key && keystate[key_rt] ? 1.0f : 0.0f;

		// Right axis comes from mouse cursor
		int rel_x, rel_y;
		auto buttons = SDL_GetRelativeMouseState(&rel_x, &rel_y);
		// Cap relative mouse motion at <sensitivity> pixels / frame
		clamp(rel_x, -sensitivity, sensitivity);
		clamp(rel_y, -sensitivity, sensitivity);
		rx = normalize(rel_x, sensitivity);
		ry = -normalize(rel_y, sensitivity);

		int abs_x, abs_y;
		SDL_GetMouseState(&abs_x, &abs_y);
		rxa = (float)abs_x;
		rya = (float)abs_y;

		update_buttons(buttons);
	}

	void KeyboardDevice::update_buttons(Uint32 buttons)
	{
		std::array<bool, VirtualButton::_Count> states;
		std::fill(states.begin(), states.end(), false);

		// Update & handle mouse buttons
		for (auto i = 0u; i < mouse_buttons.size(); i++)
		{
			mouse_buttons[i] = (buttons & SDL_BUTTON(SDL_BUTTON_LEFT + i)) > 0;
			if (mouse_mapping[i] > no_key)
				states[mouse_mapping[i]] |= mouse_buttons[i];
		}

		// Handle mapped keys
		for (auto i = 0u; i < VirtualButton::_Count; i++)
		{
			if (mapping[i] > no_key)
				states[i] |= keystate[mapping[i]] == 1;
		}

		for (auto i = 0u; i < VirtualButton::_Count; i++)
			update_button_state(static_cast<VirtualButton>(i), states[i]);
	}
	
	std::string KeyboardDevice::get_button_name(VirtualButton button) const
	{
		if (button == mouse_mapping[0])
			return "Left Mouse Button";
		else if (button == mouse_mapping[1])
			return "Middle Mouse Button";
		else if (button == mouse_mapping[2])
			return "Right Mouse Button";
		else if (button == mouse_mapping[3])
			return "Extra Mouse Button 1";
		else if (button == mouse_mapping[4])
			return "Extra Mouse Button 2";

		const auto code = mapping[button];
		if (code == no_key)
			return "n/a";
		else
			return SDL_GetScancodeName(static_cast<SDL_Scancode>(code));
	}
}