#pragma once

#ifdef XBOX_SUPPORT

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <Xinput.h>
#include "inputdevice.h"

namespace dukat
{
	class XBoxDevice : public InputDevice
	{
	private:
		const static float sensitivity;

		const int joystick_index;
		int joystick_id;
		DWORD last_package;
		XINPUT_STATE state;
		void normalize_axis(SHORT ix, SHORT iy, float& ox, float& oy, SHORT deadzone = 0);
		void normalize_trigger(BYTE i, float& o, BYTE deadzone = 0);

	public:
		XBoxDevice(Window* window, int joystick_index);
		~XBoxDevice(void);
		void update(void);
		bool is_pressed(VirtualButton button) const;
		int id(void) const { return joystick_id; }
		std::string get_button_name(VirtualButton button) const;
	};
}
#endif