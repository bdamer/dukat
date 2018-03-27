#include "stdafx.h"
#include "xboxdevice.h"
#include "mathutil.h"
#include "window.h"

#ifdef XBOX_SUPPORT

namespace dukat
{
	const float XBoxDevice::sensitivity = 1.0f;

	XBoxDevice::XBoxDevice(Window* window, SDL_JoystickID id) : InputDevice(window, id, false)
	{
		ZeroMemory(&state, sizeof(XINPUT_STATE));
		mapping[VirtualButton::Button1] = XINPUT_GAMEPAD_A;
        mapping[VirtualButton::Button2] = XINPUT_GAMEPAD_B;
		mapping[VirtualButton::Button3] = XINPUT_GAMEPAD_X;
		mapping[VirtualButton::Button4] = XINPUT_GAMEPAD_Y;
		mapping[VirtualButton::Button5] = XINPUT_GAMEPAD_LEFT_SHOULDER;
		mapping[VirtualButton::Button6] = XINPUT_GAMEPAD_RIGHT_SHOULDER;
		mapping[VirtualButton::Menu] = XINPUT_GAMEPAD_START;
		mapping[VirtualButton::Pause] = XINPUT_GAMEPAD_BACK;
		mapping[VirtualButton::Debug1] = XINPUT_GAMEPAD_DPAD_UP;
		mapping[VirtualButton::Debug2] = XINPUT_GAMEPAD_DPAD_RIGHT;
		mapping[VirtualButton::Debug3] = XINPUT_GAMEPAD_DPAD_DOWN;
		mapping[VirtualButton::Debug4] = XINPUT_GAMEPAD_DPAD_LEFT;
	}

	XBoxDevice::~XBoxDevice(void)
	{
	}

	void XBoxDevice::update(void)
	{
		if (XInputGetState((DWORD)id, &state) != ERROR_SUCCESS)
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
			for (int i = 0; i < VirtualButton::_Count; i++)
			{
				update_button_state((VirtualButton)i, (state.Gamepad.wButtons & mapping[i]) != 0);
			}
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

	void XBoxDevice::normalize_axis(SHORT ix, SHORT iy, float& ox, float& oy, SHORT deadzone)
	{
		// determine how far the controller is pushed
		float magnitude = sqrtf((float)(ix * ix + iy * iy));
		if (magnitude <= deadzone)
		{
			ox = 0.0f;
			oy = 0.0f;
		}
		else
		{
			ox = normalize((short)ix);
			oy = normalize((short)iy);
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