#include "stdafx.h"
#include "gamepaddevice.h"
#include "log.h"
#include "mathutil.h"
#include "settings.h"
#include "window.h"

namespace dukat
{
	const float GamepadDevice::sensitivity = 1.0f;

	GamepadDevice::GamepadDevice(Window* window, SDL_JoystickID id, const Settings& settings) : InputDevice(window, id, false)
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
		mapping[VirtualButton::Debug1] = settings.get_int("input.gamepad.debug1", 12);	// DPAD Down
		mapping[VirtualButton::Debug2] = settings.get_int("input.gamepad.debug2", 14);	// DPAD Right
		mapping[VirtualButton::Debug3] = settings.get_int("input.gamepad.debug3", 13);	// DPAD Left
		mapping[VirtualButton::Debug4] = settings.get_int("input.gamepad.debug4", 11);	// DPAD Up

		if (!SDL_IsGameController(id))
		{
			log->warn("Attempting to use incompatible device as gamepad: {}", id);
		}

		device = SDL_GameControllerOpen(id);
		if (device == nullptr)
		{
			std::ostringstream ss;
			ss << "Could not open gamepad: " << SDL_GetError();
			throw std::runtime_error(ss.str());
		}
		name = SDL_GameControllerName(device);
		log->info("Gamepad connected: {}", name);

		SDL_JoystickGUID guid = SDL_JoystickGetDeviceGUID(id);
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

		for (int i = 0; i < VirtualButton::_Count; i++)
		{
			if (mapping[i] < 0)
				continue;
			update_button_state((VirtualButton)i, SDL_GameControllerGetButton(device, static_cast<SDL_GameControllerButton>(mapping[i])) != 0);
		}

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
}