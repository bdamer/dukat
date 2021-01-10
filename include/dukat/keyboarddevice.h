#pragma once

#include "inputdevice.h"

namespace dukat
{
	class Settings;
	class Window;

	class KeyboardDevice : public InputDevice
	{
	private:
		// state
		int num_keys;
		const Uint8* keystate;
		std::array<bool, 5> mouse_buttons;
		// Maps mouse buttons to virtual keys.
		std::array<int, 5> mouse_mapping;
		// Mouse sensitivity - 1 is the most sensitive, higher values reduce sensitivity
		int sensitivity;
		// Updates button states.
		void update_buttons(Uint32 buttons);

	public:
		static constexpr auto keyboard_id = -1;

		KeyboardDevice(Window* window, const Settings& settings);
		~KeyboardDevice(void);
		void update(void);
		void set_sensitivity(int sensitivity) { this->sensitivity = sensitivity; }
		bool is_pressed(VirtualButton button) const;
		int id(void) const { return keyboard_id; }
		std::string get_button_name(VirtualButton button) const;
	};
}