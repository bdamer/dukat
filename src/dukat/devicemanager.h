#pragma once

#include <SDL2/SDL.h>
#include <memory>
#include <vector>

namespace dukat
{
	class InputDevice;
	class Window;

	class DeviceManager
	{
	private:
		std::vector<std::unique_ptr<InputDevice>> controllers;

	public:
		InputDevice* active;
		bool joystick_support;

		DeviceManager(void) : joystick_support(true) { }
		~DeviceManager(void) { }

		void add_keyboard(Window* window);
		void remove_keyboard(void);
		void add_joystick(Window* window, SDL_JoystickID id);
		void remove_joystick(SDL_JoystickID id);
		void update(void);
	};
}