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
		if (!SDL_IsGameController(device_index))
			log->warn("Attempting to use incompatible device as gamepad: {}", device_index);

		device = SDL_GameControllerOpen(device_index);
		if (device == nullptr)
		{
			std::ostringstream ss;
			ss << "Could not open gamepad: " << SDL_GetError();
			throw std::runtime_error(ss.str());
		}
		name = SDL_GameControllerName(device);
		log->info("Gamepad connected: {} [{}]", name, device_index);

#if SDL_VERSION_ATLEAST(2, 0, 18)
		log->debug("Rumble support: {}", SDL_GameControllerHasRumble(device));
		log->debug("Rumble triggers support: {}", SDL_GameControllerHasRumbleTriggers(device));
#endif

		invert_y = settings.get_bool(settings::input_gamepad_inverty, true);
		deadzone = static_cast<int16_t>(settings.get_int(settings::input_gamepad_deadzone, 8000));

		restore_mapping(settings, default_profile);
	}

	GamepadDevice::~GamepadDevice(void)
	{
		if (device != nullptr)
		{
			SDL_GameControllerClose(device);
			device = nullptr;
		}
	}

	void GamepadDevice::restore_mapping(const Settings& settings, const std::string& profile)
	{
		const auto prefix = settings::input_profile_prefix + profile + ".";
		mapping[VirtualButton::Button1] = settings.get_int(prefix + "gamepad.button1", SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
		mapping[VirtualButton::Button2] = settings.get_int(prefix + "gamepad.button2", SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
		mapping[VirtualButton::Button3] = settings.get_int(prefix + "gamepad.button3", SDL_CONTROLLER_BUTTON_A);
		mapping[VirtualButton::Button4] = settings.get_int(prefix + "gamepad.button4", SDL_CONTROLLER_BUTTON_B);
		mapping[VirtualButton::Button5] = settings.get_int(prefix + "gamepad.button5", SDL_CONTROLLER_BUTTON_X);
		mapping[VirtualButton::Button6] = settings.get_int(prefix + "gamepad.button6", SDL_CONTROLLER_BUTTON_Y);
		mapping[VirtualButton::Button7] = settings.get_int(prefix + "gamepad.button7", SDL_CONTROLLER_BUTTON_LEFTSTICK);
		mapping[VirtualButton::Button8] = settings.get_int(prefix + "gamepad.button8", SDL_CONTROLLER_BUTTON_RIGHTSTICK);
		mapping[VirtualButton::Select] = settings.get_int(prefix + "gamepad.select", SDL_CONTROLLER_BUTTON_BACK);
		mapping[VirtualButton::Start] = settings.get_int(prefix + "gamepad.start", SDL_CONTROLLER_BUTTON_START);
		mapping[VirtualButton::Down] = settings.get_int(prefix + "gamepad.down", SDL_CONTROLLER_BUTTON_DPAD_DOWN);
		mapping[VirtualButton::Right] = settings.get_int(prefix + "gamepad.right", SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
		mapping[VirtualButton::Left] = settings.get_int(prefix + "gamepad.left", SDL_CONTROLLER_BUTTON_DPAD_LEFT);
		mapping[VirtualButton::Up] = settings.get_int(prefix + "gamepad.up", SDL_CONTROLLER_BUTTON_DPAD_UP);
	}

	void GamepadDevice::normalize_axis(int16_t ix, int16_t iy, float& ox, float& oy, int16_t deadzone)
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

	void GamepadDevice::update(void)
	{
		if (device == nullptr)
			return;

		normalize_axis(
			SDL_GameControllerGetAxis(device, SDL_CONTROLLER_AXIS_LEFTX),
			SDL_GameControllerGetAxis(device, SDL_CONTROLLER_AXIS_LEFTY),
			lx, ly, deadzone);
		normalize_axis(
			SDL_GameControllerGetAxis(device, SDL_CONTROLLER_AXIS_RIGHTX),
			SDL_GameControllerGetAxis(device, SDL_CONTROLLER_AXIS_RIGHTY),
			rx, ry, deadzone);
		lt = normalize(SDL_GameControllerGetAxis(device, SDL_CONTROLLER_AXIS_TRIGGERLEFT));
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
#if SDL_VERSION_ATLEAST(2, 0, 9)
		const auto lo = static_cast<uint16_t>(low_freq * 65535.0f);
		const auto hi = static_cast<uint16_t>(hi_freq * 65535.0f);
		const auto duration_ms = static_cast<uint32_t>(1000.0f * duration);
		if (SDL_GameControllerRumble(device, lo, hi, duration_ms) != 0)
			log->warn("Device doesn't support rumble: {}", name);
#endif
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
		case SDL_CONTROLLER_BUTTON_A:
			return "A";
		case SDL_CONTROLLER_BUTTON_B:
			return "B";
		case SDL_CONTROLLER_BUTTON_X:
			return "X";
		case SDL_CONTROLLER_BUTTON_Y:
			return "Y";
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
			return "Left Button";
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
			return "Right Button";
		case SDL_CONTROLLER_BUTTON_LEFTSTICK:
			return "Left Thumb";
		case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
			return "Right Thumb";
		case SDL_CONTROLLER_BUTTON_BACK:
			return "Back";
		case SDL_CONTROLLER_BUTTON_GUIDE:
			return "Guide";
		case SDL_CONTROLLER_BUTTON_START:
			return "Start";
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
			return "DPAD Down";
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
			return "DPAD Right";
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
			return "DPAD Left";
		/*
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
			return "DPAD Up";
		case SDL_CONTROLLER_BUTTON_MISC1:
			return "Misc";
		case SDL_CONTROLLER_BUTTON_PADDLE1:
			return "Paddle 1";
		case SDL_CONTROLLER_BUTTON_PADDLE2:
			return "Paddle 2";
		case SDL_CONTROLLER_BUTTON_PADDLE3:
			return "Paddle 3";
		case SDL_CONTROLLER_BUTTON_PADDLE4:
			return "Paddle 4";
		case SDL_CONTROLLER_BUTTON_TOUCHPAD:
			return "Touchpad";
		*/
		default:
			return "n/a";
		}
	}
}
