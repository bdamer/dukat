#include "stdafx.h"
#include <dukat/keyboarddevice.h>
#include <dukat/window.h>
#include <dukat/mathutil.h>
#include <dukat/sysutil.h>
#include <dukat/settings.h>

namespace dukat
{
	KeyboardDevice::KeyboardDevice(Window* window, const Settings& settings) : InputDevice(window, true)
	{
		name = "keyboard";
		sensitivity = settings.get_int("input.mouse.sensitivity", 2);
		// Initialize key mapping
		mapping[VirtualButton::Button1] = settings.get_int("input.keyboard.button1", -1);
		mapping[VirtualButton::Button2] = settings.get_int("input.keyboard.button2", -1);
		mapping[VirtualButton::Button3] = settings.get_int("input.keyboard.button3", SDL_SCANCODE_SPACE);
		mapping[VirtualButton::Button4] = settings.get_int("input.keyboard.button4", -1);
		mapping[VirtualButton::Button5] = settings.get_int("input.keyboard.button5", -1);
		mapping[VirtualButton::Button6] = settings.get_int("input.keyboard.button6", -1);
		mapping[VirtualButton::Button7] = settings.get_int("input.keyboard.button7", -1);
		mapping[VirtualButton::Button8] = settings.get_int("input.keyboard.button8", -1);
		mapping[VirtualButton::Down] = settings.get_int("input.keyboard.down", SDL_SCANCODE_S);
		mapping[VirtualButton::Right] = settings.get_int("input.keyboard.right", SDL_SCANCODE_D);
		mapping[VirtualButton::Left] = settings.get_int("input.keyboard.left", SDL_SCANCODE_A);
		mapping[VirtualButton::Up] = settings.get_int("input.keyboard.up", SDL_SCANCODE_W);
		mapping[VirtualButton::LeftTrigger] = settings.get_int("input.keyboard.lt", SDL_SCANCODE_TAB);
		mapping[VirtualButton::RightTrigger] = settings.get_int("input.keyboard.rt", SDL_SCANCODE_SPACE);
		mapping[VirtualButton::Select] = SDL_SCANCODE_ESCAPE;
		mapping[VirtualButton::Start] = SDL_SCANCODE_RETURN;
		mapping[VirtualButton::Debug] = settings.get_int("input.keyboard.debug", SDL_SCANCODE_F1);
		// Initialize mouse mapping
		mouse_mapping[0] = settings.get_int("input.mouse.left", VirtualButton::Button1);
		mouse_mapping[1] = settings.get_int("input.mouse.middle", -1);
		mouse_mapping[2] = settings.get_int("input.mouse.right", VirtualButton::Button2);
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
		const auto key_left = mapping[VirtualButton::Left];
		const auto key_right = mapping[VirtualButton::Right];
		if (key_left > -1 && key_right > -1)
		{
			lx = keystate[key_left] ? -1.0f : (keystate[key_right] ? 1.0f : 0.0f);
		}
		const auto key_up = mapping[VirtualButton::Up];
		const auto key_down = mapping[VirtualButton::Down];
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
		rx = normalize(rel_x, sensitivity);
		ry = -normalize(rel_y, sensitivity);
		
		int abs_x, abs_y;
		SDL_GetMouseState(&abs_x, &abs_y);
		rxa = (float)abs_x;
		rya = (float)abs_y;

		// Mouse buttons
		mouse_lb = (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) > 0;
		mouse_rb = (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) > 0;

		// Handle mapped keys & mouse buttons
		for (int i = 0; i < VirtualButton::_Count; i++)
		{
			if (mouse_mapping[0] == i) // left button
				update_button_state(static_cast<VirtualButton>(mouse_mapping[0]), mouse_lb);
            else if (mouse_mapping[2] == i) // right button
				update_button_state(static_cast<VirtualButton>(mouse_mapping[2]), mouse_rb);
			else if (mapping[i] > -1)
				update_button_state(static_cast<VirtualButton>(i), keystate[mapping[i]] == 1);
		}
	}

	bool KeyboardDevice::is_pressed(VirtualButton button) const
	{
		if (mouse_mapping[0] == button)
			return mouse_lb;
        else if (mouse_mapping[2] == button)
            return mouse_rb;
		else
			return keystate[mapping[button]] == 1;
	}
}