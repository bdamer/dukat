#include "stdafx.h"
#include <dukat/devicemanager.h>
#include <dukat/log.h>
#include <dukat/keyboarddevice.h>
#include <dukat/gamepaddevice.h>
#include <dukat/settings.h>

#ifdef XBOX_SUPPORT
#include <dukat/xboxdevice.h>
#endif

namespace dukat
{
	void DeviceManager::add_keyboard(Window* window)
	{
		log->info("Keyboard device added.");
		controllers.push_back(std::make_unique<KeyboardDevice>(window, settings));
		active = controllers.back().get();
		trigger(Message{ Events::DeviceBound });
	}

	void DeviceManager::remove_keyboard(void)
	{
		trigger(Message{ Events::DeviceUnbound });
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

#ifdef XBOX_SUPPORT
		const std::string xinput = "XInput Controller";
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
		trigger(Message{ Events::DeviceUnbound });
		active = controllers.back().get();
		trigger(Message{ Events::DeviceBound });
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
		trigger(Message{ Events::DeviceUnbound });
		active = controllers.back().get();
		trigger(Message{ Events::DeviceBound });
	}

	void DeviceManager::update(void)
	{
		if (active != nullptr)
		{
			active->update();
		}
	}
}