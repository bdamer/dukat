#pragma once

#include "inputdevice.h"

namespace dukat
{
	class Settings;

	// Wrapper for generic SDL gamepad.
	class GamepadDevice : public InputDevice
	{
	private:
		static constexpr auto button_a = 0;
		static constexpr auto button_b = 1;
		static constexpr auto button_x = 2;
		static constexpr auto button_y = 3;
		static constexpr auto button_select = 4;
		static constexpr auto button_start = 6;
		static constexpr auto button_left_thumb = 7;
		static constexpr auto button_right_thumb = 8;
		static constexpr auto button_left_shoulder = 9;
		static constexpr auto button_right_shoulder = 10;
		static constexpr auto dpad_up = 11;
		static constexpr auto dpad_down = 12;
		static constexpr auto dpad_left = 13;
		static constexpr auto dpad_right = 14;
		static constexpr float sensitivity = 1.0f;

		SDL_GameController* device;
		// For some reason generic SDL inverts y axis - this fixes it.
		bool invert_y;

	public:
		GamepadDevice(const Window& window, const Settings& settings, int device_index);
		~GamepadDevice(void);

		int id(void) const;
		bool is_inverted(void) const { return invert_y; }
		std::string get_button_name(VirtualButton button) const;
		void update(void);

		// Controls haptic feedback
		void start_feedback(float low_freq, float hi_freq, float duration);
		void cancel_feedback(void) { start_feedback(0.0f, 0.0f, 0.0f); }
	};
}