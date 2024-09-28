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

		void normalize_axis(int16_t ix, int16_t iy, float& ox, float& oy, int16_t deadzone);

	public:
		GamepadDevice(const Window& window, const Settings& settings, int device_index);
		~GamepadDevice(void) override;
		void restore_mapping(const Settings& settings, const std::string& profile) override;

		int id(void) const override;
		bool is_inverted(void) const { return invert_y; }
		std::string get_button_name(VirtualButton button) const override;
		void update(void) override;

		// Controls haptic feedback
		void start_feedback(float low_freq, float hi_freq, float duration) override;
		void cancel_feedback(void) override { start_feedback(0.0f, 0.0f, 0.0f); }
	};
}