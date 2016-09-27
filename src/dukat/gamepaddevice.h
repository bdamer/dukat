#pragma once

#include "inputdevice.h"

namespace dukat
{
	// Wrapper for generic SDL gamepad.
	class GamepadDevice : public InputDevice
	{
	private:
		SDL_Joystick* joystick;

	public:
		GamepadDevice(SDL_JoystickID id);
		~GamepadDevice(void);

		void update(void);
		bool is_pressed(VirtualButton button) const;
	};
}