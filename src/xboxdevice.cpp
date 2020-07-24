#include "stdafx.h"
#include <dukat/xboxdevice.h>
#include <dukat/log.h>
#include <dukat/mathutil.h>
#include <dukat/window.h>

#ifdef XBOX_SUPPORT

namespace dukat
{
	const float XBoxDevice::sensitivity = 1.0f;

	XBoxDevice::XBoxDevice(Window* window, int joystick_index) : InputDevice(window, false), joystick_index(joystick_index)
	{
		name = SDL_JoystickNameForIndex(joystick_index);
		log->debug("Initializing XInput device: {}", name);
		
		// Temporarily open SDL joystick so we can get instance ID
		auto joystick = SDL_JoystickOpen(joystick_index);
		if (joystick == nullptr)
		{
			std::ostringstream ss;
			ss << "Could not open joystick: " << SDL_GetError();
			throw std::runtime_error(ss.str());
		}
		joystick_id = SDL_JoystickInstanceID(joystick);
		SDL_JoystickClose(joystick);

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

		SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(joystick_index);
		char buffer[33];
		SDL_JoystickGetGUIDString(guid, buffer, 33);
		log->debug("Device GUID: {}", buffer);
	}

	XBoxDevice::~XBoxDevice(void)
	{
	}

	void XBoxDevice::update(void)
	{
		if (XInputGetState(static_cast<DWORD>(joystick_index), &state) != ERROR_SUCCESS)
		{
			return; // could not poll
		}
		if (state.dwPacketNumber != last_package)
		{
			last_package = state.dwPacketNumber;
			normalize_axis(state.Gamepad.sThumbLX, state.Gamepad.sThumbLY, lx, ly, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
			normalize_axis(state.Gamepad.sThumbRX, state.Gamepad.sThumbRY, rx, ry, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
			normalize_trigger(state.Gamepad.bLeftTrigger, lt, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
			normalize_trigger(state.Gamepad.bRightTrigger, rt, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
			for (auto i = 0; i < VirtualButton::LeftTrigger; i++)
			{
				update_button_state(static_cast<VirtualButton>(i), (state.Gamepad.wButtons & mapping[i]) != 0);
			}
			const auto trigger_threshold = 0xff - XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
			update_button_state(VirtualButton::LeftTrigger, state.Gamepad.bLeftTrigger > trigger_threshold);
			update_button_state(VirtualButton::RightTrigger, state.Gamepad.bRightTrigger > trigger_threshold);
		}
		// compute absolute positions
		lxa += lx * sensitivity;
		lya += ly * sensitivity;
		rxa += rx * sensitivity;
		rya -= ry * sensitivity;
		clamp(lxa, 0.0f, static_cast<float>(window->get_width()));
		clamp(lya, 0.0f, static_cast<float>(window->get_height()));
		clamp(rxa, 0.0f, static_cast<float>(window->get_width()));
		clamp(rya, 0.0f, static_cast<float>(window->get_height()));
	}

	bool XBoxDevice::is_pressed(VirtualButton button) const
	{
		return ((state.Gamepad.wButtons & mapping[button]) != 0);
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
			else
				return "n/a";
		}
	}

	void XBoxDevice::normalize_axis(SHORT ix, SHORT iy, float& ox, float& oy, SHORT deadzone)
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

	void XBoxDevice::normalize_trigger(BYTE i, float& o, BYTE deadzone)
	{
		if (i <= deadzone)
		{
			o = 0.0f;
		}
		else
		{
			o = normalize(i);
		}
	}
}
#endif