#include "stdafx.h"
#include "devicemanager.h"
#include "log.h"
#include "keyboarddevice.h"
#include "gamepaddevice.h"
#include "settings.h"

#ifdef XBOX_SUPPORT
#include "xboxdevice.h"
#endif

namespace dukat
{
	void DeviceManager::add_keyboard(Window* window)
	{
		log->info("Keyboard device added.");
		controllers.push_back(std::make_unique<KeyboardDevice>(window, settings));
		active = controllers.back().get();
	}

	void DeviceManager::remove_keyboard(void)
	{
		// TODO: implement	
	}

	void DeviceManager::add_joystick(Window* window, SDL_JoystickID id)
	{
		if (!settings.get_bool("input.joystick.support", true))
			return;

		const std::string name = SDL_JoystickNameForIndex(id);
		log->info("Joystick device [{}] added: {}", id, name);
		
		if (active != nullptr && active->id != KeyboardDevice::keyboard_id)
		{
			log->warn("Already using joystick device, ignoring new device.");
			return;
		}

		// TODO: figure out how we can detect this properly
		SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(id);
		char buffer[33];
		SDL_JoystickGetGUIDString(guid, buffer, 33);
		log->debug("Device GUID: {}", buffer);
		
#ifdef XBOX_SUPPORT
		std::string xinput = "XInput Controller";
		if (name.rfind(xinput, 0) == 0 || 
			name == "Controller (Xbox 360 Wireless Receiver for Windows)")
		{
			controllers.push_back(std::make_unique<XBoxDevice>(window, id));
		}
		else
		{
#endif
			controllers.push_back(std::make_unique<GamepadDevice>(window, id, settings));
#ifdef XBOX_SUPPORT
		}
#endif
		active = controllers.back().get();
	}

	void DeviceManager::remove_joystick(SDL_JoystickID id)
	{
		if (!settings.get_bool("input.joystick.support", true))
			return;
		log->info("Joystick device removed: {}", id);
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