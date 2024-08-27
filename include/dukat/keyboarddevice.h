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

		KeyboardDevice(const Window& window, const Settings& settings);
		~KeyboardDevice(void) override;

		int get_mapping(VirtualButton button) const override;
		bool is_mapped(int key, VirtualButton& button) const override;
		void override_mapping(VirtualButton button, int key) override;
		void clear_mapping(VirtualButton button) override;
		void restore_mapping(const Settings& settings) override;

		void update(void) override;
		void set_sensitivity(int sensitivity) { this->sensitivity = sensitivity; }
		int id(void) const override { return keyboard_id; }
		std::string get_button_name(VirtualButton button) const override;
		static int mouse_button_to_key(const int mb) { return -(mb + 1); }
	};
}