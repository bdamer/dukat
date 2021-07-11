#pragma once

#include <dukat/inputstate.h>

namespace dukat
{
	class InputDevice;

	class InputRecorder
	{
	private:
		enum ControlFlag
		{
			Buttons = 1,
			LX = 2,
			LY = 4,
			RX = 8,
			RY = 16,
			LT = 32,
			RT = 64
		};

		int32_t device_id;

		std::unique_ptr<std::ofstream> os;
		std::unique_ptr<std::ifstream> is;

	public:
		InputRecorder(void) : os(nullptr), is(nullptr), device_id(0) { }
		~InputRecorder(void) { }

		void start_recording(const std::string& filename, int device_id);
		void stop_recording(void);
		void start_playback(const std::string& filename);
		void stop_playback(void);

		void record_frame(const InputState& state);
		void replay_frame(InputState& state);

		int get_device_id(void) const { return device_id; }
	};
}
