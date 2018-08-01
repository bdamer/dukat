#include "stdafx.h"
#include "gamepaddevice.h"
#include "log.h"
#include "mathutil.h"
#include "window.h"

namespace dukat
{
	const float GamepadDevice::sensitivity = 1.0f;

	GamepadDevice::GamepadDevice(Window* window, SDL_JoystickID id) : InputDevice(window, id, false)
	{
		mapping[VirtualButton::Button1] = 9;
        mapping[VirtualButton::Button2] = 10;
		mapping[VirtualButton::Button3] = 0;
		mapping[VirtualButton::Button4] = 1;
		mapping[VirtualButton::Button5] = 2;
		mapping[VirtualButton::Button6] = 3;
		mapping[VirtualButton::Button7] = 7;
		mapping[VirtualButton::Button8] = 8;
		mapping[VirtualButton::Select] = 4;
		mapping[VirtualButton::Start] = 6;
		mapping[VirtualButton::Debug1] = -1;
		mapping[VirtualButton::Debug2] = -1;
		mapping[VirtualButton::Debug3] = -1;
		mapping[VirtualButton::Debug4] = -1;

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
		log->info("Gamepad connected: {}", SDL_GameControllerName(device));
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
		ly = -normalize(SDL_GameControllerGetAxis(device, SDL_CONTROLLER_AXIS_LEFTY));
		lt = normalize(SDL_GameControllerGetAxis(device, SDL_CONTROLLER_AXIS_TRIGGERLEFT));
		rx = normalize(SDL_GameControllerGetAxis(device, SDL_CONTROLLER_AXIS_RIGHTX));
		ry = -normalize(SDL_GameControllerGetAxis(device, SDL_CONTROLLER_AXIS_RIGHTY));
		rt = normalize(SDL_GameControllerGetAxis(device, SDL_CONTROLLER_AXIS_TRIGGERRIGHT));

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