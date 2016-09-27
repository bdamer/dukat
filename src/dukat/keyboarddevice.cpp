#include "stdafx.h"
#include "keyboarddevice.h"
#include "window.h"
#include "mathutil.h"
#include "sysutil.h"

namespace dukat
{
	// TODO: this is actually mouse sensitivity - the smaller the value, the more 
	// sensitive mouse movement it.
	const float KeyboardDevice::mouse_range = 200.0f;

	KeyboardDevice::KeyboardDevice(Window* window) : InputDevice(keyboard_id, true), window(window)
	{
		mapping[VirtualButton::Pause] = SDL_SCANCODE_P;
		mapping[VirtualButton::Debug1] = SDL_SCANCODE_GRAVE;
		mapping[VirtualButton::Debug2] = SDL_SCANCODE_2;
		mapping[VirtualButton::Debug3] = SDL_SCANCODE_3;
		mapping[VirtualButton::Debug4] = SDL_SCANCODE_4;
		keystate = SDL_GetKeyboardState(&num_keys);
		sdl_check_result(SDL_SetRelativeMouseMode(SDL_TRUE), "Set mouse mode");
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
		int mx, my;
		auto buttons = SDL_GetRelativeMouseState(&mx, &my);
		mouse_x += mx;
		mouse_y += my;
		// normalize screen coordinates
		rx = (float)mouse_x / mouse_range;
		ry = (float)-mouse_y / mouse_range;

		if (normalization == Dependent)
		{
			// Normalize both axis based on radidus
			float r = sqrt(rx * rx + ry * ry);
			if (r > 1.0)
			{
				rx /= r;
				ry /= r;
				mouse_x = (int)(rx * mouse_range);
				mouse_y = (int)(-ry * mouse_range);
			}
		}
		else
		{
			// Normalize each axis independently
			clamp(rx, -1.0f, 1.0f);
			clamp(ry, -1.0f, 1.0f);
			mouse_x = (int)(rx * mouse_range);
			mouse_y = (int)(-ry * mouse_range);
		}


		// Mouse buttons
		mouse_lb = (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) > 0;
		mouse_rb = (buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) > 0;

		// Handle mapped keys & mouse buttons
		for (int i = 0; i < VirtualButton::_Count; i++)
		{
			if (VirtualButton::PrimaryAction == (VirtualButton)i)
			{
				udapte_button_state(VirtualButton::PrimaryAction, mouse_lb);
			}
            else if (VirtualButton::SecondaryAction == (VirtualButton)i)
            {
                udapte_button_state(VirtualButton::SecondaryAction, mouse_rb);
            }
			else
			{
				udapte_button_state((VirtualButton)i, keystate[mapping[i]] == 1);
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