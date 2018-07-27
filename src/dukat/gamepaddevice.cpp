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
		mapping[VirtualButton::Button1] = 1;
        mapping[VirtualButton::Button2] = 2;
		mapping[VirtualButton::Button3] = 3;
		mapping[VirtualButton::Button4] = 4;
		mapping[VirtualButton::Debug1] = -1;
		mapping[VirtualButton::Debug2] = -1;
		mapping[VirtualButton::Debug3] = -1;
		mapping[VirtualButton::Debug4] = -1;
		joystick = SDL_JoystickOpen(id);
		if (joystick == nullptr)
		{
			std::ostringstream ss;
			ss << "Could not open joystick: " << SDL_GetError();
			throw std::runtime_error(ss.str());
		}
		log->debug("Gamepad connected: {} {} axes, {} buttons", SDL_JoystickName(joystick), 
			SDL_JoystickNumAxes(joystick), SDL_JoystickNumButtons(joystick));
	}

	GamepadDevice::~GamepadDevice(void)
	{
		if (joystick != nullptr)
		{
			SDL_JoystickClose(joystick);
			joystick = nullptr;
		}
	}

	void GamepadDevice::update(void)
	{
		if (joystick == nullptr)
		{
			return;
		}
		// Read axis
		lx = normalize(SDL_JoystickGetAxis(joystick, 0));
		ly = normalize(SDL_JoystickGetAxis(joystick, 1));
		lt = SDL_JoystickGetButton(joystick, 6);
		rx = normalize(SDL_JoystickGetAxis(joystick, 2));
		ry = normalize(SDL_JoystickGetAxis(joystick, 3));
		rt = SDL_JoystickGetButton(joystick, 7);
		for (int i = 0; i < VirtualButton::_Count; i++)
		{
			update_button_state((VirtualButton)i, SDL_JoystickGetButton(joystick, mapping[i]) != 0);
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
		return SDL_JoystickGetButton(joystick, mapping[button]) == SDL_PRESSED;
	}
}