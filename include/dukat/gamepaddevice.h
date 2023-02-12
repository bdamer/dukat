#pragma once

#include "inputdevice.h"

namespace dukat
{
	class Settings;

	// Wrapper for generic SDL gamepad.
	class GamepadDevice : public InputDevice
	{
	private:
		static constexpr float sensitivity = 1.0f;

		SDL_GameController* device;
		int16_t deadzone;
		// For some reason generic SDL inverts y axis - this fixes it.
		bool invert_y;

		void initialize_mapping(const Settings& settings);
		void normalize_axis(int16_t ix, int16_t iy, float& ox, float& oy, int16_t deadzone);

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