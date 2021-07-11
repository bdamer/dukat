#include "stdafx.h"
#include <dukat/playbackdevice.h>
#include <dukat/inputrecorder.h>
#include <dukat/log.h>
#include <dukat/settings.h>

namespace dukat
{
	PlaybackDevice::PlaybackDevice(const Window& window, const Settings& settings, InputRecorder* recorder) 
		: InputDevice(window, settings, true), recorder(recorder), playback_id(recorder->get_device_id())
	{
	}

	void PlaybackDevice::update(void)
	{
		InputState state;
		recorder->replay_frame(state);

		lx = state.lx;
		ly = state.ly;
		rx = state.rx;
		ry = state.ry;
		lt = state.lt;
		rt = state.rt;

		for (auto i = 0u; i < VirtualButton::_Count; i++)
			update_button_state(static_cast<VirtualButton>(i), state.buttons[i]);
	}
}