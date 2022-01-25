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
		// Initialize key mapping
		mapping[VirtualButton::Button1] = settings.get_int("input.keyboard.button1", -1);
		mapping[VirtualButton::Button2] = settings.get_int("input.keyboard.button2", -1);
		mapping[VirtualButton::Button3] = settings.get_int("input.keyboard.button3", -1);
		mapping[VirtualButton::Button4] = settings.get_int("input.keyboard.button4", -1);
		mapping[VirtualButton::Button5] = settings.get_int("input.keyboard.button5", -1);
		mapping[VirtualButton::Button6] = settings.get_int("input.keyboard.button6", -1);
		mapping[VirtualButton::Button7] = settings.get_int("input.keyboard.button7", -1);
		mapping[VirtualButton::Button8] = settings.get_int("input.keyboard.button8", -1);
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
		// Initialize default key bindings
		keystate = SDL_GetKeyboardState(&num_keys);
	}

	KeyboardDevice::~KeyboardDevice(void)
	{
		SDL_SetRelativeMouseMode(SDL_FALSE);
		keystate = nullptr; // memory handled by SDL
	}

	void KeyboardDevice::update(void)
	{
		SDL_PumpEvents();

		// Handle direction keys
		const auto key_left = mapping[VirtualButton::LeftAxisLeft];
		const auto key_right = mapping[VirtualButton::LeftAxisRight];
		if (key_left > -1 && key_right > -1)
		{
			lx = keystate[key_left] ? -1.0f : (keystate[key_right] ? 1.0f : 0.0f);
		}
		const auto key_up = mapping[VirtualButton::LeftAxisUp];
		const auto key_down = mapping[VirtualButton::LeftAxisDown];
		if (key_up > -1 && key_down > -1)
		{
			ly = keystate[key_up] ? 1.0f : (keystate[key_down] ? -1.0f : 0.0f);
		}

		// Handle trigger keys
		const auto key_lt = mapping[VirtualButton::LeftTrigger];
		lt = key_lt > -1 && keystate[key_lt] ? 1.0f : 0.0f;
		const auto key_rt = mapping[VirtualButton::RightTrigger];
		rt = key_rt > -1 && keystate[key_rt] ? 1.0f : 0.0f;

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
			if (mouse_mapping[i] > -1)
				states[mouse_mapping[i]] |= mouse_buttons[i];
		}

		// Handle mapped keys
		for (auto i = 0u; i < VirtualButton::_Count; i++)
		{
			if (mapping[i] > -1)
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
		if (code == -1)
			return "n/a";
		else
			return SDL_GetScancodeName(static_cast<SDL_Scancode>(code));
	}
}