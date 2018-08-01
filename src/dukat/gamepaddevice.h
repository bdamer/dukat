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

	public:
		// For some reason generic SDL inverts y axis - this fixes it.
		bool invert_y;

		GamepadDevice(Window* window, SDL_JoystickID id, const Settings& settings);
		~GamepadDevice(void);

		void update(void);
		bool is_pressed(VirtualButton button) const;
	};
}