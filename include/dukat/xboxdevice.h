#pragma once

#ifdef XINPUT_SUPPORT

#define NOMINMAX

#include <Windows.h>
#include <Xinput.h>
#include "inputdevice.h"

namespace dukat
{
	class XBoxDevice : public InputDevice
	{
	private:
		static std::array<bool, XUSER_MAX_COUNT> user_slots;
		const static float sensitivity;
		int device_index;
		int device_id;
		DWORD last_package;
		XINPUT_STATE state;

		void select_device_index(void);
		bool check_device_caps(int index) const;
		void initialize_mapping(void);
		void normalize_axis(int16_t ix, int16_t iy, float& ox, float& oy, int16_t deadzone = 0);
		void normalize_trigger(uint8_t i, float& o, uint8_t deadzone = 0);

	public:
		XBoxDevice(const Window& window, const Settings& settings, int device_index);
		~XBoxDevice(void);

		int id(void) const { return device_id; }
		std::string get_button_name(VirtualButton button) const;
		void update(void);

		// Controls haptic feedback
		void start_feedback(float low_freq, float hi_freq, float duration);
		void cancel_feedback(void) { start_feedback(0.0f, 0.0f, 0.0f); }
	};

	bool is_xinput_device(int device_index);
}
#endif