#pragma once

#include <memory>
#include <vector>
#include "messenger.h"

namespace dukat
{
	class InputDevice;
	class InputRecorder;
	class Settings;
	class Window;

	class DeviceManager : public Messenger
	{
	private:
		const Window& window;
		const Settings& settings;
		std::vector<std::unique_ptr<InputDevice>> controllers;
		std::unique_ptr<InputRecorder> recorder;
		bool recording;
		bool replaying;

	public:
		InputDevice* active;

		DeviceManager(const Window& window, const Settings& settings) : window(window), settings(settings), 
			recorder(nullptr), recording(false), replaying(false), active(nullptr) { }
		~DeviceManager(void) { }

		void add_keyboard(void);
		void remove_keyboard(void);
		void add_joystick(int index);
		void remove_joystick(SDL_JoystickID id);
		void update(void);

		void start_record(const std::string& filename);
		void stop_record(void);
		bool is_recording(void) const { return recording; }

		void start_replay(const std::string& filename);
		void stop_replay(void);
		bool is_replaying(void) const { return replaying; }
	};
}