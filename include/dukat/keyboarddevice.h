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

		// Returns key or mouse index assigned to a given virtual button in current mapping
		int get_mapping(VirtualButton button) const override;
		// Returns key or mouse index assigned to a given virtual button in specific profile.
		int get_profile_mapping(VirtualButton button, const std::string& profile, const Settings& settings) const;
		// Returns mouse button index assigned to a given virtual button in current mapping.
		int get_mouse_mapping(VirtualButton button) const;
		int get_mouse_profile_mapping(VirtualButton button, const std::string& profile, const Settings& settings) const;
		// Returns keyboard index assigned to a given virtual button in current mapping.
		int get_key_mapping(VirtualButton button) const;
		// Returns keyboard index assigned to a given virtual button in a specific profile.
		int get_key_profile_mapping(VirtualButton button, const std::string& profile, const Settings& settings) const;

		bool is_mapped(int key, VirtualButton& button) const override;
		bool is_mapped(int key, const std::string& profile, Settings& settings, VirtualButton& button) const;
		void override_mapping(VirtualButton button, int key) override;
		void override_profile_mapping(VirtualButton button, int key, const std::string& profile, Settings& settings) const;
		void clear_mapping(VirtualButton button) override;
		void clear_profile_mapping(VirtualButton button, const std::string& profile, Settings& settings) const;
		void restore_mapping(const Settings& settings, const std::string& profile) override;

		void update(void) override;
		void set_sensitivity(int sensitivity) { this->sensitivity = sensitivity; }
		int id(void) const override { return keyboard_id; }
		std::string get_button_name(VirtualButton button) const override;
		static int mouse_button_to_key(const int mb) { return -(mb + 1); }
		static int key_to_mouse_button(const int key) { return -key - 1; }
	};
}