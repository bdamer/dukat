#include "stdafx.h"
#include "keyboarddevice.h"
#include "window.h"
#include "mathutil.h"
#include "sysutil.h"

namespace dukat
{

	KeyboardDevice::KeyboardDevice(Window* window) : InputDevice(window, keyboard_id, true), sensitivity(2)
	{
		mapping[VirtualButton::Jump] = SDL_SCANCODE_SPACE;
		mapping[VirtualButton::Pause] = SDL_SCANCODE_P;
		mapping[VirtualButton::Debug1] = SDL_SCANCODE_GRAVE;
		mapping[VirtualButton::Debug2] = SDL_SCANCODE_2;
		mapping[VirtualButton::Debug3] = SDL_SCANCODE_3;
		mapping[VirtualButton::Debug4] = SDL_SCANCODE_4;
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
			if (VirtualButton::PrimaryAction == (VirtualButton)i)
			{
				update_button_state(VirtualButton::PrimaryAction, mouse_lb);
			}
            else if (VirtualButton::SecondaryAction == (VirtualButton)i)
            {
				update_button_state(VirtualButton::SecondaryAction, mouse_rb);
            }
			else
			{
				update_button_state((VirtualButton)i, keystate[mapping[i]] == 1);
			}
		}
	}

	bool KeyboardDevice::is_pressed(VirtualButton button) const
	{
		if (VirtualButton::PrimaryAction == button)
		{
			return mouse_lb;
		}
        else if (VirtualButton::SecondaryAction == button)
        {
            return mouse_rb;
        }
		else
		{
			return keystate[mapping[button]] == 1;
		}
	}
}