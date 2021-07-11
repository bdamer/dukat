#include "stdafx.h"
#include <dukat/devicemanager.h>
#include <dukat/inputrecorder.h>
#include <dukat/log.h>
#include <dukat/keyboarddevice.h>
#include <dukat/gamepaddevice.h>
#include <dukat/playbackdevice.h>
#include <dukat/mathutil.h>
#include <dukat/settings.h>

#ifdef XBOX_SUPPORT
#include <dukat/xboxdevice.h>
#endif

namespace dukat
{
	void DeviceManager::add_keyboard(void)
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

	void DeviceManager::add_joystick(int joystick_index)
	{
		if (!settings.get_bool("input.joystick.support", true))
			return;

		const std::string name = SDL_JoystickNameForIndex(joystick_index);
		log->info("Joystick device @ index [{}] added: {}", joystick_index, name);
		
		if (active != nullptr && active->id() != KeyboardDevice::keyboard_id)
		{
			log->warn("Already using joystick device, ignoring new device.");
			return;
		}

#ifdef XBOX_SUPPORT
		const std::string xinput = "XInput Controller";
		if (name.rfind(xinput, 0) == 0 || 
			name == "Controller (Xbox 360 Wireless Receiver for Windows)")
		{
			controllers.push_back(std::make_unique<XBoxDevice>(window, settings, joystick_index));
		}
		else
		{
#endif
			controllers.push_back(std::make_unique<GamepadDevice>(window, settings, joystick_index));
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
			if (controllers[i]->id() == id)
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
			if (recording && recorder != nullptr)
				recorder->record_frame(active->get_state());
		}
	}

	void DeviceManager::start_record(const std::string& filename)
	{
		assert(recorder == nullptr);

		log->info("Starting recording: {}", filename);
		recorder = std::make_unique<InputRecorder>();
		recorder->start_recording(filename, active->id());
		recording = true;
	}

	void DeviceManager::stop_record(void)
	{
		if (recording)
		{
			log->info("Stopping recording");
			recorder = nullptr;
			recording = false;
		}
	}

	void DeviceManager::start_replay(const std::string& filename)
	{
		assert(recorder == nullptr);

		log->info("Starting replay: {}", filename);
		recorder = std::make_unique<InputRecorder>();
		recorder->start_playback(filename);

		controllers.push_back(std::make_unique<PlaybackDevice>(window, settings, recorder.get()));
		active = controllers.back().get();
		replaying = true;

		trigger(Message{ Events::DeviceBound });
	}
	
	void DeviceManager::stop_replay(void)
	{
		if (replaying)
		{
			log->info("Stopping replay");
			recorder = nullptr;
			controllers.pop_back();
			active = controllers.back().get();
			replaying = false;
		}
	}
}