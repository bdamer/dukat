#include "stdafx.h"
#include "devicemanager.h"
#include "log.h"
#include "keyboarddevice.h"
#include "gamepaddevice.h"

#ifdef XBOX_SUPPORT
#include "xboxdevice.h"
#endif

namespace dukat
{
	void DeviceManager::add_keyboard(Window* window)
	{
		logger << "Keyboard device added." << std::endl;
		controllers.push_back(std::make_unique<KeyboardDevice>(window));
		active = controllers.back().get();
	}

	void DeviceManager::remove_keyboard(void)
	{
		// TODO: implement	
	}

	void DeviceManager::add_joystick(Window* window, SDL_JoystickID id)
	{
		if (!joystick_support)
			return;

		logger << "Joystick device added: " << id << std::endl;
#ifdef XBOX_SUPPORT
		const std::string name = SDL_JoystickNameForIndex(id);
		// TODO: figure out how we can detect this properly
		SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(id);
		char buffer[33];
		SDL_JoystickGetGUIDString(guid, buffer, 33);
		logger << "Device: " << name << " " << buffer << std::endl;
		if (name == "Controller (Xbox 360 Wireless Receiver for Windows)" ||
			name == "XInput Controller #1")
		{
			controllers.push_back(std::make_unique<XBoxDevice>(window, id));;
		}
		else
		{
			controllers.push_back(std::make_unique<GamepadDevice>(window, id));;
		}
#else
		controllers.push_back(std::make_unique<GamepadDevice>(window, id));;
#endif
		active = controllers.back().get();
	}

	void DeviceManager::remove_joystick(SDL_JoystickID id)
	{
		if (!joystick_support)
			return;
		logger << "Joystick device removed: " << id << std::endl;
		std::vector<std::unique_ptr<InputDevice>>::size_type i = 0;
		while (i < controllers.size())
		{
			if (controllers[i]->id == id)
			{
				controllers.erase(controllers.begin() + i);
				break;
			}
			++i;
		}
		active = controllers.back().get();
	}

	void DeviceManager::update(void)
	{
		if (active != nullptr)
		{
			active->update();
		}
	}
}