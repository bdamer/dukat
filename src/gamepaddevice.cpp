#include "stdafx.h"
#include <dukat/gamepaddevice.h>
#include <dukat/log.h>
#include <dukat/mathutil.h>
#include <dukat/settings.h>
#include <dukat/window.h>

namespace dukat
{
	const float GamepadDevice::sensitivity = 1.0f;

	GamepadDevice::GamepadDevice(Window* window, int joystick_index, const Settings& settings) : InputDevice(window, false)
	{
		invert_y = settings.get_bool("input.gamepad.inverty", true);

		mapping[VirtualButton::Button1] = settings.get_int("input.gamepad.button1", 9);	// Left Button
        mapping[VirtualButton::Button2] = settings.get_int("input.gamepad.button2", 10);// Right Button
		mapping[VirtualButton::Button3] = settings.get_int("input.gamepad.button3", 0);	// A
		mapping[VirtualButton::Button4] = settings.get_int("input.gamepad.button4", 1);	// B
		mapping[VirtualButton::Button5] = settings.get_int("input.gamepad.button5", 2);	// X
		mapping[VirtualButton::Button6] = settings.get_int("input.gamepad.button6", 3);	// Y
		mapping[VirtualButton::Button7] = settings.get_int("input.gamepad.button7", 7); // Left Thumb
		mapping[VirtualButton::Button8] = settings.get_int("input.gamepad.button8", 8); // Right Tumb
		mapping[VirtualButton::Select] = settings.get_int("input.gamepad.select", 4); 	// Select Button
		mapping[VirtualButton::Start] = settings.get_int("input.gamepad.start", 6);		// Start Button
		mapping[VirtualButton::Down] = settings.get_int("input.gamepad.down", 12);		// DPAD Down
		mapping[VirtualButton::Right] = settings.get_int("input.gamepad.right", 14);	// DPAD Right
		mapping[VirtualButton::Left] = settings.get_int("input.gamepad.left", 13);		// DPAD Left
		mapping[VirtualButton::Up] = settings.get_int("input.gamepad.up", 11);			// DPAD Up

		if (!SDL_IsGameController(joystick_index))
		{
			log->warn("Attempting to use incompatible device as gamepad: {}", joystick_index);
		}

		device = SDL_GameControllerOpen(joystick_index);
		if (device == nullptr)
		{
			std::ostringstream ss;
			ss << "Could not open gamepad: " << SDL_GetError();
			throw std::runtime_error(ss.str());
		}
		name = SDL_GameControllerName(device);
		log->info("Gamepad connected: {}", name);

		SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(joystick_index);
		char buffer[33];
		SDL_JoystickGetGUIDString(guid, buffer, 33);
		log->debug("Device GUID: {}", buffer);
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

		// compute absolute positions
		lxa += lx * sensitivity;
		lya += ly * sensitivity;
		rxa += rx * sensitivity;
		rya += ry * sensitivity;
		clamp(lxa, 0.0f, static_cast<float>(window->get_width()));
		clamp(lya, 0.0f, static_cast<float>(window->get_height()));
		clamp(rxa, 0.0f, static_cast<float>(window->get_width()));
		clamp(rya, 0.0f, static_cast<float>(window->get_height()));
	}

	bool GamepadDevice::is_pressed(VirtualButton button) const
	{
		return SDL_GameControllerGetButton(device, static_cast<SDL_GameControllerButton>(mapping[button])) == SDL_PRESSED;
	}

	int GamepadDevice::id(void) const
	{
		auto joystick = SDL_GameControllerGetJoystick(device);
		return SDL_JoystickInstanceID(joystick);
	}
}