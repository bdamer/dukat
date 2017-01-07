#include "stdafx.h"
#include "gamepaddevice.h"
#include "log.h"
#include "mathutil.h"

namespace dukat
{
	GamepadDevice::GamepadDevice(SDL_JoystickID id) : InputDevice(id, false)
	{
		mapping[VirtualButton::PrimaryAction] = 1;
        mapping[VirtualButton::SecondaryAction] = 2;
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
		logger << "Gamepad connected: " << SDL_JoystickName(joystick) << " " <<
			SDL_JoystickNumAxes(joystick) << " axes, " << SDL_JoystickNumButtons(joystick) << " buttons" << std::endl;
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
	}

	bool GamepadDevice::is_pressed(VirtualButton button) const
	{
		return SDL_JoystickGetButton(joystick, mapping[button]) == SDL_PRESSED;
	}
}