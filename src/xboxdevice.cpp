#include "stdafx.h"
#include <dukat/xboxdevice.h>
#include <dukat/log.h>
#include <dukat/mathutil.h>
#include <dukat/window.h>

#ifdef XINPUT_SUPPORT

namespace dukat
{
	std::array<bool, XUSER_MAX_COUNT> XBoxDevice::user_slots = { false, false, false, false };
	const float XBoxDevice::sensitivity = 1.0f;

	XBoxDevice::XBoxDevice(const Window& window, const Settings& settings, int device_index) 
		: InputDevice(window, settings, false), device_index(-1)
	{
		name = SDL_JoystickNameForIndex(device_index);
		log->debug("XInput device connected: {} [{}]", name, device_index);
		
		// Temporarily open SDL joystick so we can get instance ID
		auto device = SDL_JoystickOpen(device_index);
		if (device == nullptr)
		{
			std::ostringstream ss;
			ss << "Could not open joystick: " << SDL_GetError();
			throw std::runtime_error(ss.str());
		}
		device_id = SDL_JoystickInstanceID(device);
		SDL_JoystickClose(device);

		select_device_index();
		restore_mapping(settings, default_profile);
	}

	XBoxDevice::~XBoxDevice(void)
	{
		if (device_index > -1)
			user_slots[device_index] = false;
	}

	void XBoxDevice::select_device_index(void)
	{
		for (auto i = 0; i < XUSER_MAX_COUNT; i++)
		{
			if (user_slots[i])
				continue; // slot already taken
			if (check_device_caps(i))
			{
				device_index = i;
				user_slots[i] = true; // mark slot as used
				break; // we've found our index
			}
		}
		if (device_index == -1)
			log->warn("Unable to find index for xinput device.");
		else
			log->debug("Using xinput device @ {}", device_index);
	}

	bool XBoxDevice::check_device_caps(int index) const
	{
		XINPUT_CAPABILITIES caps;
		ZeroMemory(&caps, sizeof(XINPUT_CAPABILITIES));
		if (XInputGetCapabilities(static_cast<DWORD>(index), XINPUT_FLAG_GAMEPAD, &caps) != ERROR_SUCCESS)
			return false;

		std::stringstream ss;
		if (caps.Flags & XINPUT_CAPS_VOICE_SUPPORTED)
			ss << "XINPUT_CAPS_VOICE_SUPPORTED|";
		// Device caps reported depend on XInput version
#if(_WIN32_WINNT >= _WIN32_WINNT_WIN8)
		if (caps.Flags & XINPUT_CAPS_FFB_SUPPORTED)
			ss << "XINPUT_CAPS_FFB_SUPPORTED|";
		if (caps.Flags & XINPUT_CAPS_WIRELESS)
			ss << "XINPUT_CAPS_WIRELESS|";
		if (caps.Flags & XINPUT_CAPS_PMD_SUPPORTED)
			ss << "XINPUT_CAPS_PMD_SUPPORTED|";
		if (caps.Flags & XINPUT_CAPS_NO_NAVIGATION)
			ss << "XINPUT_CAPS_NO_NAVIGATION|";
#endif
		log->debug("Device caps: {}", ss.str());

		return true;
	}

	void XBoxDevice::restore_mapping(const Settings& settings, const std::string& profile)
	{
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		mapping[VirtualButton::Button1] = XINPUT_GAMEPAD_LEFT_SHOULDER;
		mapping[VirtualButton::Button2] = XINPUT_GAMEPAD_RIGHT_SHOULDER;
		mapping[VirtualButton::Button3] = XINPUT_GAMEPAD_A;
		mapping[VirtualButton::Button4] = XINPUT_GAMEPAD_B;
		mapping[VirtualButton::Button5] = XINPUT_GAMEPAD_X;
		mapping[VirtualButton::Button6] = XINPUT_GAMEPAD_Y;
		mapping[VirtualButton::Button7] = XINPUT_GAMEPAD_LEFT_THUMB;
		mapping[VirtualButton::Button8] = XINPUT_GAMEPAD_RIGHT_THUMB;
		mapping[VirtualButton::Select] = XINPUT_GAMEPAD_BACK;
		mapping[VirtualButton::Start] = XINPUT_GAMEPAD_START;
		mapping[VirtualButton::Down] = XINPUT_GAMEPAD_DPAD_DOWN;
		mapping[VirtualButton::Right] = XINPUT_GAMEPAD_DPAD_RIGHT;
		mapping[VirtualButton::Left] = XINPUT_GAMEPAD_DPAD_LEFT;
		mapping[VirtualButton::Up] = XINPUT_GAMEPAD_DPAD_UP;
	}

	void XBoxDevice::update(void)
	{
		if (XInputGetState(static_cast<DWORD>(device_index), &state) != ERROR_SUCCESS)
			return; // could not poll

		// Update button states
		last_package = state.dwPacketNumber;
		normalize_axis(state.Gamepad.sThumbLX, state.Gamepad.sThumbLY, lx, ly, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		normalize_axis(state.Gamepad.sThumbRX, state.Gamepad.sThumbRY, rx, ry, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		normalize_trigger(state.Gamepad.bLeftTrigger, lt, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
		normalize_trigger(state.Gamepad.bRightTrigger, rt, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
		for (auto i = 0; i < VirtualButton::LeftTrigger; i++)
			update_button_state(static_cast<VirtualButton>(i), (state.Gamepad.wButtons & mapping[i]) != 0);

		const auto trigger_threshold = 0xff - XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
		update_button_state(VirtualButton::LeftTrigger, state.Gamepad.bLeftTrigger > trigger_threshold);
		update_button_state(VirtualButton::RightTrigger, state.Gamepad.bRightTrigger > trigger_threshold);
		update_button_state(VirtualButton::LeftAxisDown, ly <= -1.0f);
		update_button_state(VirtualButton::LeftAxisUp, ly >= 1.0f);
		update_button_state(VirtualButton::LeftAxisLeft, lx <= -1.0f);
		update_button_state(VirtualButton::LeftAxisRight, lx >= 1.0f);

		// compute absolute positions
		lxa += lx * sensitivity;
		lya += ly * sensitivity;
		rxa += rx * sensitivity;
		rya -= ry * sensitivity;
		clamp(lxa, 0.0f, static_cast<float>(window.get_width()));
		clamp(lya, 0.0f, static_cast<float>(window.get_height()));
		clamp(rxa, 0.0f, static_cast<float>(window.get_width()));
		clamp(rya, 0.0f, static_cast<float>(window.get_height()));
	}

	std::string XBoxDevice::get_button_name(VirtualButton button) const
	{
		switch (mapping[button])
		{
		case XINPUT_GAMEPAD_LEFT_SHOULDER:
			return "Left Button";
		case XINPUT_GAMEPAD_RIGHT_SHOULDER:
			return "Right Button";
		case XINPUT_GAMEPAD_A:
			return "A";
		case XINPUT_GAMEPAD_B:
			return "B";
		case XINPUT_GAMEPAD_X:
			return "X";
		case XINPUT_GAMEPAD_Y:
			return "Y";
		case XINPUT_GAMEPAD_LEFT_THUMB:
			return "Left Thumb";
		case XINPUT_GAMEPAD_RIGHT_THUMB:
			return "Right Thumb";
		case XINPUT_GAMEPAD_BACK:
			return "Select";
		case XINPUT_GAMEPAD_START:
			return "Start";
		case XINPUT_GAMEPAD_DPAD_DOWN:
			return "DPAD Down";
		case XINPUT_GAMEPAD_DPAD_RIGHT:
			return "DPAD Right";
		case XINPUT_GAMEPAD_DPAD_LEFT:
			return "DPAD Left";
		case XINPUT_GAMEPAD_DPAD_UP:
			return "DPAD Up";
		default:
			if (button == VirtualButton::LeftTrigger)
				return "Left Trigger";
			else if (button == VirtualButton::RightTrigger)
				return "Right Trigger";
			else if (button == VirtualButton::LeftAxisDown)
				return "LEFT Down";
			else if (button == VirtualButton::LeftAxisLeft)
				return "LEFT Left";
			else if (button == VirtualButton::LeftAxisRight)
				return "LEFT Right";
			else if (button == VirtualButton::LeftAxisUp)
				return "LEFT Up";
			else
				return "n/a";
		}
	}

	void XBoxDevice::start_feedback(float low_freq, float hi_freq, float duration)
	{
		// We are ignoring duration - client will have to call cancel_feedback to stop effect
		XINPUT_VIBRATION vibration;
		ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
		vibration.wLeftMotorSpeed = static_cast<uint16_t>(low_freq * 65535.0f);
		vibration.wRightMotorSpeed = static_cast<uint16_t>(hi_freq * 65535.0f);
		XInputSetState(static_cast<DWORD>(device_index), &vibration);
	}

	void XBoxDevice::normalize_axis(int16_t ix, int16_t iy, float& ox, float& oy, int16_t deadzone)
	{
		// determine how far the controller is pushed
		const auto magnitude = std::sqrt(static_cast<float>(ix * ix + iy * iy));
		if (magnitude <= deadzone)
		{
			ox = 0.0f;
			oy = 0.0f;
		}
		else
		{
			ox = normalize(static_cast<short>(ix));
			oy = normalize(static_cast<short>(iy));
		}
	}

	void XBoxDevice::normalize_trigger(uint8_t i, float& o, uint8_t deadzone)
	{
		o = (i <= deadzone) ? 0.0f : normalize(i);
	}

	bool is_xinput_device(int device_index)
	{
		const std::string device_name = SDL_JoystickNameForIndex(device_index);
		const std::string xinput = "XInput Controller";
		return device_name == "Controller (Xbox 360 Wireless Receiver for Windows)"
			|| device_name == "Controller (XBOX 360 For Windows)"
			|| device_name == "Xbox 360 Controller"
			|| device_name.rfind(xinput, 0) == 0;
	}
}
#endif