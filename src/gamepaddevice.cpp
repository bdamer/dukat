#include "stdafx.h"
#include <dukat/gamepaddevice.h>
#include <dukat/log.h>
#include <dukat/mathutil.h>
#include <dukat/settings.h>
#include <dukat/window.h>

namespace dukat
{
	GamepadDevice::GamepadDevice(const Window& window, const Settings& settings, int device_index) 
		: InputDevice(window, settings, false)
	{
		invert_y = settings.get_bool("input.gamepad.inverty", true);

		mapping[VirtualButton::Button1] = settings.get_int("input.gamepad.button1", button_left_shoulder);
		mapping[VirtualButton::Button2] = settings.get_int("input.gamepad.button2", button_right_shoulder);
		mapping[VirtualButton::Button3] = settings.get_int("input.gamepad.button3", button_a);
		mapping[VirtualButton::Button4] = settings.get_int("input.gamepad.button4", button_b);
		mapping[VirtualButton::Button5] = settings.get_int("input.gamepad.button5", button_x);
		mapping[VirtualButton::Button6] = settings.get_int("input.gamepad.button6", button_y);
		mapping[VirtualButton::Button7] = settings.get_int("input.gamepad.button7", button_left_thumb);
		mapping[VirtualButton::Button8] = settings.get_int("input.gamepad.button8", button_right_thumb);
		mapping[VirtualButton::Select] = settings.get_int("input.gamepad.select", button_select);
		mapping[VirtualButton::Start] = settings.get_int("input.gamepad.start", button_start);
		mapping[VirtualButton::Down] = settings.get_int("input.gamepad.down", dpad_down);
		mapping[VirtualButton::Right] = settings.get_int("input.gamepad.right", dpad_right);
		mapping[VirtualButton::Left] = settings.get_int("input.gamepad.left", dpad_left);
		mapping[VirtualButton::Up] = settings.get_int("input.gamepad.up", dpad_up);

		if (!SDL_IsGameController(device_index))
		{
			log->warn("Attempting to use incompatible device as gamepad: {}", device_index);
		}

		device = SDL_GameControllerOpen(device_index);
		if (device == nullptr)
		{
			std::ostringstream ss;
			ss << "Could not open gamepad: " << SDL_GetError();
			throw std::runtime_error(ss.str());
		}
		name = SDL_GameControllerName(device);
		log->info("Gamepad connected: {}", name);

		SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(device_index);
		char buffer[33];
		SDL_JoystickGetGUIDString(guid, buffer, 33);
		log->debug("Device GUID: {}", buffer);
		log->debug("Rumble support: {}", SDL_GameControllerHasRumble(device));
		log->debug("Rumble triggers support: {}", SDL_GameControllerHasRumbleTriggers(device));
	}

	GamepadDevice::~GamepadDevice(void)
	{
		if (device != nullptr)
		{
			SDL_GameControllerClose(device);
			device = nullptr;
		}
	}

	void GamepadDevice::update(void)
	{
		if (device == nullptr)
			return;

		lx = normalize(SDL_GameControllerGetAxis(device, SDL_CONTROLLER_AXIS_LEFTX));
		ly = normalize(SDL_GameControllerGetAxis(device, SDL_CONTROLLER_AXIS_LEFTY));
		lt = normalize(SDL_GameControllerGetAxis(device, SDL_CONTROLLER_AXIS_TRIGGERLEFT));
		rx = normalize(SDL_GameControllerGetAxis(device, SDL_CONTROLLER_AXIS_RIGHTX));
		ry = normalize(SDL_GameControllerGetAxis(device, SDL_CONTROLLER_AXIS_RIGHTY));
		rt = normalize(SDL_GameControllerGetAxis(device, SDL_CONTROLLER_AXIS_TRIGGERRIGHT));

		if (invert_y)
		{
			ly = -ly;
			ry = -ry;
		}

		for (auto i = 0; i < VirtualButton::LeftTrigger; i++)
		{
			if (mapping[i] < 0)
				continue;
			update_button_state(static_cast<VirtualButton>(i), 
				SDL_GameControllerGetButton(device, static_cast<SDL_GameControllerButton>(mapping[i])) != 0);
		}
		update_button_state(VirtualButton::LeftTrigger, lt > 0.0f);
		update_button_state(VirtualButton::RightTrigger, rt > 0.0f);
		update_button_state(VirtualButton::LeftAxisDown, ly <= -1.0f);
		update_button_state(VirtualButton::LeftAxisUp, ly >= 1.0f);
		update_button_state(VirtualButton::LeftAxisLeft, lx <= -1.0f);
		update_button_state(VirtualButton::LeftAxisRight, lx >= 1.0f);

		// compute absolute positions
		lxa += lx * sensitivity;
		lya += ly * sensitivity;
		rxa += rx * sensitivity;
		rya += ry * sensitivity;
		clamp(lxa, 0.0f, static_cast<float>(window.get_width()));
		clamp(lya, 0.0f, static_cast<float>(window.get_height()));
		clamp(rxa, 0.0f, static_cast<float>(window.get_width()));
		clamp(rya, 0.0f, static_cast<float>(window.get_height()));
	}

	void GamepadDevice::start_feedback(float low_freq, float hi_freq, float duration)
	{
		const auto lo = static_cast<uint16_t>(low_freq * 65535.0f);
		const auto hi = static_cast<uint16_t>(hi_freq * 65535.0f);
		const auto duration_ms = static_cast<uint32_t>(1000.0f * duration);
		if (SDL_GameControllerRumble(device, lo, hi, duration_ms) != 0)
			log->warn("Device doesn't support rumble: {}", name);
	}

	int GamepadDevice::id(void) const
	{
		auto joystick = SDL_GameControllerGetJoystick(device);
		return SDL_JoystickInstanceID(joystick);
	}

	std::string GamepadDevice::get_button_name(VirtualButton button) const
	{
		switch (mapping[button])
		{
		case button_a:
			return "A";
		case button_b:
			return "B";
		case button_x:
			return "X";
		case button_y:
			return "Y";
		case button_left_shoulder:
			return "Left Button";
		case button_right_shoulder:
			return "Right Button";
		case button_left_thumb:
			return "Left Thumb";
		case button_right_thumb:
			return "Right Thumb";
		case button_select:
			return "Select";
		case button_start:
			return "Start";
		case dpad_down:
			return "DPAD Down";
		case dpad_right:
			return "DPAD Right";
		case dpad_left:
			return "DPAD Left";
		case dpad_up:
			return "DPAD Up";
		default:
			return "n/a";
		}
	}
}