#include "stdafx.h"
#include <dukat/keyboarddevice.h>
#include <dukat/log.h>
#include <dukat/window.h>
#include <dukat/mathutil.h>
#include <dukat/sysutil.h>
#include <dukat/settings.h>

namespace dukat
{
	struct KeyMapping
	{
		std::string path; // setting path
		int default_value;
	};

	static const std::unordered_map<InputDevice::VirtualButton, KeyMapping> key_mappings = {
		std::make_pair(InputDevice::Button1, KeyMapping{ "keyboard.button1", InputDevice::no_key }),
		std::make_pair(InputDevice::Button2, KeyMapping{ "keyboard.button2", InputDevice::no_key }),
		std::make_pair(InputDevice::Button3, KeyMapping{ "keyboard.button3", InputDevice::no_key }),
		std::make_pair(InputDevice::Button4, KeyMapping{ "keyboard.button4", InputDevice::no_key }),
		std::make_pair(InputDevice::Button5, KeyMapping{ "keyboard.button5", InputDevice::no_key }),
		std::make_pair(InputDevice::Button6, KeyMapping{ "keyboard.button6", InputDevice::no_key }),
		std::make_pair(InputDevice::Button7, KeyMapping{ "keyboard.button7", InputDevice::no_key }),
		std::make_pair(InputDevice::Button8, KeyMapping{ "keyboard.button8", InputDevice::no_key }),
		std::make_pair(InputDevice::Select, KeyMapping{ "keyboard.cancel", SDL_SCANCODE_ESCAPE }),
		std::make_pair(InputDevice::Start, KeyMapping{ "keyboard.confirm", SDL_SCANCODE_RETURN }),
		std::make_pair(InputDevice::Down, KeyMapping{ "keyboard.down", SDL_SCANCODE_DOWN }),
		std::make_pair(InputDevice::Right, KeyMapping{ "keyboard.right", SDL_SCANCODE_RIGHT }),
		std::make_pair(InputDevice::Left, KeyMapping{ "keyboard.left", SDL_SCANCODE_LEFT }),
		std::make_pair(InputDevice::Up, KeyMapping{ "keyboard.up", SDL_SCANCODE_UP }),
		std::make_pair(InputDevice::LeftTrigger, KeyMapping{ "keyboard.lt", SDL_SCANCODE_TAB }),
		std::make_pair(InputDevice::RightTrigger, KeyMapping{ "keyboard.rt", SDL_SCANCODE_SPACE }),
		std::make_pair(InputDevice::LeftAxisDown, KeyMapping{ "keyboard.leftaxis.down", SDL_SCANCODE_S }),
		std::make_pair(InputDevice::LeftAxisRight, KeyMapping{ "keyboard.leftaxis.right", SDL_SCANCODE_D }),
		std::make_pair(InputDevice::LeftAxisLeft, KeyMapping{ "keyboard.leftaxis.left", SDL_SCANCODE_A }),
		std::make_pair(InputDevice::LeftAxisUp, KeyMapping{ "keyboard.leftaxis.up", SDL_SCANCODE_W }),
		std::make_pair(InputDevice::Debug, KeyMapping{ "keyboard.debug", SDL_SCANCODE_F1 }),
	};

	struct MouseMapping
	{
		std::string path;
		int default_button;
	};

	static const std::unordered_map<int, MouseMapping> mouse_mappings = {
		std::make_pair(0, MouseMapping{ "mouse.left", static_cast<int>(InputDevice::Button1) }),
		std::make_pair(1, MouseMapping{ "mouse.middle", static_cast<int>(InputDevice::Button2) }),
		std::make_pair(2, MouseMapping{ "mouse.right", static_cast<int>(InputDevice::Button3) }),
		std::make_pair(3, MouseMapping{ "mouse.extra1", static_cast<int>(InputDevice::Button4) }),
		std::make_pair(4, MouseMapping{ "mouse.extra2", static_cast<int>(InputDevice::Button5) }),
	};

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
		const auto btn = InputDevice::get_mapping(button);
		if (btn != no_key) return btn;
		for (auto i = 0; i < static_cast<int>(mouse_mapping.size()); i++)
		{
			if (mouse_mapping[i] == static_cast<int>(button))
				return mouse_button_to_key(i + 1); // map to [-2,-6]
		}
		return no_key;
	}

	int KeyboardDevice::get_profile_mapping(VirtualButton button, const std::string& profile, const Settings& settings) const
	{
		auto res = get_key_profile_mapping(button, profile, settings);
		if (res == no_key)
			res = get_mouse_profile_mapping(button, profile, settings);
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
		const auto prefix = settings::input_profile_prefix + profile + ".";
		for (auto i = 0u; i < mouse_mapping.size(); i++)
		{
			const auto& mm = mouse_mappings.at(i);
			const auto bound_btn = settings.get_int(prefix + mm.path, mm.default_button);
			if (static_cast<int>(button) == bound_btn)
				return mouse_button_to_key(i + 1);
		}
		return no_key;
	}

	int KeyboardDevice::get_key_mapping(VirtualButton button) const
	{
		return InputDevice::get_mapping(button);
	}

	int KeyboardDevice::get_key_profile_mapping(VirtualButton button, const std::string& profile, const Settings& settings) const
	{
		if (!key_mappings.count(button))
			return no_key;
		const auto prefix = settings::input_profile_prefix + profile + ".";
		const auto& km = key_mappings.at(button);
		return settings.get_int(prefix + km.path, km.default_value);
	}

	bool KeyboardDevice::is_mapped(int key, VirtualButton& button) const
	{
		if (key < InputDevice::no_key) // indicates mouse button
		{
			const auto mouse_button = key_to_mouse_button(key) - 1;
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

	bool KeyboardDevice::is_mapped(int key, const std::string& profile, Settings& settings, VirtualButton& button) const
	{
		const auto prefix = settings::input_profile_prefix + profile + ".";
		if (key < InputDevice::no_key) // indicates mouse button
		{
			const auto mouse_button = key_to_mouse_button(key) - 1;
			const auto& mm = mouse_mappings.at(mouse_button);
			const auto bound_btn = settings.get_int(prefix + mm.path, mm.default_button);
			if (bound_btn != no_key)
			{
				button = static_cast<VirtualButton>(bound_btn);
				return true;
			}
		}
		else
		{
			for (auto i = 0; i < static_cast<int>(VirtualButton::_Count); i++)
			{
				button = static_cast<VirtualButton>(i);
				const auto bound_key = get_key_profile_mapping(button, profile, settings);
				if (key == bound_key)
					return true;
			}
		}
		return false;
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

	void KeyboardDevice::override_profile_mapping(VirtualButton button, int key, const std::string& profile, Settings& settings) const
	{
		const auto prefix = settings::input_profile_prefix + profile + ".";
		if (key < no_key) // mouse button
		{
			const auto idx = key_to_mouse_button(key) - 1;
			const auto& mm = mouse_mappings.at(idx);
			settings.set(prefix + mm.path, static_cast<int>(button));
		}
		else // keyboard key
		{
			if (!key_mappings.count(button)) return; // invalid button
			const auto& km = key_mappings.at(button);
			settings.set(prefix + km.path, static_cast<int>(key));
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

	void KeyboardDevice::clear_profile_mapping(VirtualButton button, const std::string& profile, Settings& settings) const
	{
		log->debug("Clear keyboard mapping: {}", profile);

		const auto prefix = settings::input_profile_prefix + profile + ".";
		for (auto i = 0u; i < mouse_mapping.size(); i++)
		{
			const auto& mm = mouse_mappings.at(i);
			if (static_cast<int>(button) == settings.get_int(prefix + mm.path, mm.default_button))
				settings.set(prefix + mm.path, no_key);
		}
		override_profile_mapping(button, no_key, profile, settings);
	}

	void KeyboardDevice::restore_mapping(const Settings& settings, const std::string& profile)
	{
		log->debug("Restore keyboard mapping: {}", profile);

		// Initialize key mapping
		for (auto i = 0; i < static_cast<int>(VirtualButton::_Count); i++)
			mapping[i] = get_key_profile_mapping(static_cast<VirtualButton>(i), profile, settings);

		// Initialize mouse mapping
		const auto prefix = settings::input_profile_prefix + profile + ".";
		for (auto i = 0u; i < mouse_mapping.size(); i++)
		{
			const auto& mm = mouse_mappings.at(i);
			mouse_mapping[i] = settings.get_int(prefix + mm.path, mm.default_button);
		}
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