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
		sensitivity = settings.get_int(settings::input_mouse_sensitivity, 128);
		restore_mapping(settings, default_profile);
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

	int KeyboardDevice::get_profile_mapping(VirtualButton button, const std::string& profile, const Settings& settings) const
	{
		auto res = get_mouse_profile_mapping(button, profile, settings);
		if (res == no_key)
			res = get_key_profile_mapping(button, profile, settings);
		return res;
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

	int KeyboardDevice::get_mouse_profile_mapping(VirtualButton button, const std::string& profile, const Settings& settings) const
	{
		const auto prefix = "input.profiles." + profile + ".";
		if (button == settings.get_int(prefix + "mouse.left", VirtualButton::Button1))
			return mouse_button_to_key(1);
		else if (button == settings.get_int(prefix + "mouse.middle", VirtualButton::Button3))
			return mouse_button_to_key(2);
		else if (button == settings.get_int(prefix + "mouse.right", VirtualButton::Button2))
			return mouse_button_to_key(3);
		else if (button == settings.get_int(prefix + "mouse.extra1", VirtualButton::Button4))
			return mouse_button_to_key(4);
		else if (button == settings.get_int(prefix + "mouse.extra2", VirtualButton::Button5))
			return mouse_button_to_key(5);
		else
			return no_key;
	}

	int KeyboardDevice::get_key_mapping(VirtualButton button) const
	{
		return InputDevice::get_mapping(button);
	}

	int KeyboardDevice::get_key_profile_mapping(VirtualButton button, const std::string& profile, const Settings& settings) const
	{
		const auto prefix = "input.profiles." + profile + ".";
		switch (button)
		{
		case Button1:
			return settings.get_int(prefix + "keyboard.button1", no_key);
		case Button2:
			return settings.get_int(prefix + "keyboard.button2", no_key);
		case Button3:
			return settings.get_int(prefix + "keyboard.button3", no_key);
		case Button4:
			return settings.get_int(prefix + "keyboard.button4", no_key);
		case Button5:
			return settings.get_int(prefix + "keyboard.button5", no_key);
		case Button6:
			return settings.get_int(prefix + "keyboard.button6", no_key);
		case Button7:
			return settings.get_int(prefix + "keyboard.button7", no_key);
		case Button8:
			return settings.get_int(prefix + "keyboard.button8", no_key);
		case Select:
			return SDL_SCANCODE_ESCAPE;
		case Start:
			return SDL_SCANCODE_RETURN;
		case Down:
			return settings.get_int(prefix + "keyboard.down", SDL_SCANCODE_DOWN);
		case Right:
			return settings.get_int(prefix + "keyboard.right", SDL_SCANCODE_RIGHT);
		case Left:
			return settings.get_int(prefix + "keyboard.left", SDL_SCANCODE_LEFT);
		case Up:
			return settings.get_int(prefix + "keyboard.up", SDL_SCANCODE_UP);
		case LeftTrigger:
			return settings.get_int(prefix + "keyboard.lt", SDL_SCANCODE_TAB);
		case RightTrigger:
			return settings.get_int(prefix + "keyboard.rt", SDL_SCANCODE_SPACE);
		case LeftAxisDown:
			return settings.get_int(prefix + "keyboard.leftaxis.down", SDL_SCANCODE_S);
		case LeftAxisRight:
			return settings.get_int(prefix + "keyboard.leftaxis.right", SDL_SCANCODE_D);
		case LeftAxisLeft:
			return settings.get_int(prefix + "keyboard.leftaxis.left", SDL_SCANCODE_A);
		case LeftAxisUp:
			return settings.get_int(prefix + "keyboard.leftaxis.up", SDL_SCANCODE_W);
		case Debug:
			return settings.get_int(prefix + "keyboard.debug", SDL_SCANCODE_F1);
		default:
			return no_key;
		}
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

	void KeyboardDevice::restore_mapping(const Settings& settings, const std::string& profile)
	{
		// Initialize key mapping
		for (auto i = 0; i < static_cast<int>(VirtualButton::_Count); i++)
			mapping[i] = get_key_profile_mapping(static_cast<VirtualButton>(i), profile, settings);

		// Initialize mouse mapping
		const auto prefix = "input.profiles." + profile + ".";
		mouse_mapping[0] = settings.get_int(prefix + "mouse.left", VirtualButton::Button1);
		mouse_mapping[1] = settings.get_int(prefix + "mouse.middle", VirtualButton::Button3);
		mouse_mapping[2] = settings.get_int(prefix + "mouse.right", VirtualButton::Button2);
		mouse_mapping[3] = settings.get_int(prefix + "mouse.extra1", VirtualButton::Button4);
		mouse_mapping[4] = settings.get_int(prefix + "mouse.extra2", VirtualButton::Button5);
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