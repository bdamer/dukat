#pragma once

#include "inputdevice.h"

namespace dukat
{
	class InputRecorder;
	class Settings;
	class Window;

	class PlaybackDevice : public InputDevice
	{
	private:
		const int playback_id;
		InputRecorder* recorder;

	public:
		PlaybackDevice(const Window& window, const Settings& settings, InputRecorder* recorder);
		~PlaybackDevice(void) override { }
		void update(void) override;

		int id(void) const override { return playback_id; }
		std::string get_button_name(VirtualButton button) const override { return ""; }
	};
}