#pragma once

#ifdef XINPUT_SUPPORT

#define NOMINMAX

#include <Windows.h>
#include <Xinput.h>
#include "inputdevice.h"

namespace dukat
{
	class XBoxDevice : public InputDevice
	{
	private:
		const static float sensitivity;
		const int device_index;
		int device_id;
		DWORD last_package;
		XINPUT_STATE state;

		void normalize_axis(SHORT ix, SHORT iy, float& ox, float& oy, SHORT deadzone = 0);
		void normalize_trigger(BYTE i, float& o, BYTE deadzone = 0);

	public:
		XBoxDevice(const Window& window, const Settings& settings, int device_index);
		~XBoxDevice(void) { }

		int id(void) const { return device_id; }
		std::string get_button_name(VirtualButton button) const;
		void update(void);

		// Controls haptic feedback
		void start_feedback(float low_freq, float hi_freq, float duration);
		void cancel_feedback(void) { start_feedback(0.0f, 0.0f, 0.0f); }
	};
}
#endif