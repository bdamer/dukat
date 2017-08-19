#pragma once

#include "inputdevice.h"

namespace dukat
{
	// Wrapper for generic SDL gamepad.
	class GamepadDevice : public InputDevice
	{
	private:
		const static float sensitivity;
		SDL_Joystick* joystick;

	public:
		GamepadDevice(Window* window, SDL_JoystickID id);
		~GamepadDevice(void);

		void update(void);
		bool is_pressed(VirtualButton button) const;
	};
}