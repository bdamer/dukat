#include "stdafx.h"
#include <dukat/keyboarddevice.h>
#include <dukat/window.h>
#include <dukat/mathutil.h>
#include <dukat/sysutil.h>
#include <dukat/settings.h>

namespace dukat
{
	KeyboardDevice::KeyboardDevice(Window* window, const Settings& settings) : InputDevice(window, keyboard_id, true)
	{
		name = "keyboard";
		sensitivity = settings.get_int("input.mouse.sensitivity", 2);
		// Initialize key mapping
		mapping[VirtualButton::Button3] = settings.get_int("input.keyboard.button3", SDL_SCANCODE_SPACE);
		mapping[VirtualButton::Button4] = settings.get_int("input.keyboard.button4", -1);
		mapping[VirtualButton::Button5] = settings.get_int("input.keyboard.button5", -1);
		mapping[VirtualButton::Button6] = settings.get_int("input.keyboard.button6", -1);
		mapping[VirtualButton::Button7] = settings.get_int("input.keyboard.button7", -1);
		mapping[VirtualButton::Button8] = settings.get_int("input.keyboard.button8", -1);
		mapping[VirtualButton::Debug1] = settings.get_int("input.keyboard.debug1", SDL_SCANCODE_F1);
		mapping[VirtualButton::Debug2] = settings.get_int("input.keyboard.debug2", SDL_SCANCODE_F2);
		mapping[VirtualButton::Debug3] = settings.get_int("input.keyboard.debug3", SDL_SCANCODE_F3);
		mapping[VirtualButton::Debug4] = settings.get_int("input.keyboard.debug4", SDL_SCANCODE_F4);
		mapping[VirtualButton::Select] = SDL_SCANCODE_ESCAPE;
		mapping[VirtualButton::Start] = SDL_SCANCODE_RETURN;
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
		lx = keystate[SDL_SCANCODE_A] ? -1.0f :
			(keystate[SDL_SCANCODE_D] ? 1.0f : 0.0f);
		ly = keystate[SDL_SCANCODE_W] ? 1.0f :
			(keystate[SDL_SCANCODE_S] ? -1.0f : 0.0f);

		// Handle trigger keys
		lt = keystate[SDL_SCANCODE_TAB] ? 1.0f : 0.0f;

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
			if (VirtualButton::Button1 == (VirtualButton)i)
			{
				update_button_state(VirtualButton::Button1, mouse_lb);
			}
            else if (VirtualButton::Button2 == (VirtualButton)i)
            {
				update_button_state(VirtualButton::Button2, mouse_rb);
            }
			else if (mapping[i] > -1)
			{
				update_button_state((VirtualButton)i, keystate[mapping[i]] == 1);
			}
		}
	}

	bool KeyboardDevice::is_pressed(VirtualButton button) const
	{
		if (VirtualButton::Button1 == button)
		{
			return mouse_lb;
		}
        else if (VirtualButton::Button2 == button)
        {
            return mouse_rb;
        }
		else
		{
			return keystate[mapping[button]] == 1;
		}
	}
}