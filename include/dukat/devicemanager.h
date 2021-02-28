#pragma once

#include <memory>
#include <vector>
#include "messenger.h"

namespace dukat
{
	class InputDevice;
	class Settings;
	class Window;

	class DeviceManager : public Messenger
	{
	private:
		const Window& window;
		const Settings& settings;
		std::vector<std::unique_ptr<InputDevice>> controllers;

	public:
		InputDevice* active;

		DeviceManager(const Window& window, const Settings& settings) : window(window), settings(settings) { }
		~DeviceManager(void) { }

		void add_keyboard(void);
		void remove_keyboard(void);
		void add_joystick(int index);
		void remove_joystick(SDL_JoystickID id);
		void update(void);
	};
}