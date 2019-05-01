#pragma once

#include "inputdevice.h"

namespace dukat
{
	class Settings;

	// Wrapper for generic SDL gamepad.
	class GamepadDevice : public InputDevice
	{
	private:
		const static float sensitivity;
		SDL_GameController* device;
		// For some reason generic SDL inverts y axis - this fixes it.
		bool invert_y;

	public:
		GamepadDevice(Window* window, int joystick_index, const Settings& settings);
		~GamepadDevice(void);

		void update(void);
		bool is_pressed(VirtualButton button) const;
		bool is_inverted(void) const { return invert_y; }
		int id(void) const;
	};
}