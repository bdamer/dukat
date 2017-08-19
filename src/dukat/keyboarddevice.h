#pragma once

#include "inputdevice.h"

namespace dukat
{
	class Window;

	class KeyboardDevice : public InputDevice
	{
	private:
		int num_keys;
		const Uint8* keystate;
		bool mouse_lb, mouse_rb;
		// Mouse sensitivity - 1 is the most sensitive, higher values reduce sensitivity
		int sensitivity;

	public:
		static const int keyboard_id = -1;

		KeyboardDevice(Window* window);
		~KeyboardDevice(void);
		void update(void);

		void set_sensitivity(int sensitivity) { this->sensitivity = sensitivity; }
		bool is_pressed(VirtualButton button) const;
	};
}