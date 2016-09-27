#pragma once

#include "inputdevice.h"

namespace dukat
{
	class Window;

	class KeyboardDevice : public InputDevice
	{
	private:
		// Radius of mouse range around center of screen. 
		const static float mouse_range;

		Window* window;
		int num_keys;
		const Uint8* keystate;
		bool mouse_lb, mouse_rb;
		int mouse_x, mouse_y;

	public:
		static const int keyboard_id = -1;

		KeyboardDevice(Window* window);
		~KeyboardDevice(void);
		void update(void);

		bool is_pressed(VirtualButton button) const;
	};
}