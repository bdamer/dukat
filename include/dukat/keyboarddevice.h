#pragma once

#include "inputdevice.h"

namespace dukat
{
	class Settings;
	class Window;

	class KeyboardDevice : public InputDevice
	{
	private:
		// key mappings
		SDL_Scancode key_left;
		SDL_Scancode key_right;
		SDL_Scancode key_up;
		SDL_Scancode key_down;
		SDL_Scancode key_lt;
		SDL_Scancode key_rt;
		// state
		int num_keys;
		const Uint8* keystate;
		bool mouse_lb, mouse_rb;
		// Mouse sensitivity - 1 is the most sensitive, higher values reduce sensitivity
		int sensitivity;

	public:
		static constexpr auto keyboard_id = -1;

		KeyboardDevice(Window* window, const Settings& settings);
		~KeyboardDevice(void);
		void update(void);

		void set_sensitivity(int sensitivity) { this->sensitivity = sensitivity; }
		bool is_pressed(VirtualButton button) const;
	};
}