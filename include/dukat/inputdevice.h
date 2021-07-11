#pragma once

#include <array>
#include <functional>
#include <map>
#include <dukat/inputstate.h>

namespace dukat
{
	class Window;
	class Settings;

	// Base class for all supported input devices
	class InputDevice
	{
	public:
		enum VirtualButton
		{
			Button1,
            Button2,
			Button3,
			Button4,
			Button5,
			Button6,
			Button7,
			Button8,
			Select,
			Start,
			Down,
			Up,
			Left,
			Right,
			LeftTrigger,
			RightTrigger,
			LeftAxisDown,
			LeftAxisUp,
			LeftAxisLeft,
			LeftAxisRight,
			Debug,
			_Count
		};

	private:
		static std::array<std::function<void(void)>, VirtualButton::_Count> button_handlers;
		static std::array<std::function<void(void)>, VirtualButton::_Count> long_press_handlers;
		std::array<Uint32, VirtualButton::_Count> buttons; // tracks current button state

	protected:
		std::string name;
		const Window& window;
		Uint32 long_press_threshold; // time to long press in ms
		std::array<int, VirtualButton::_Count> mapping;
		void update_button_state(VirtualButton button, bool pressed);

	public:
		// relative left axis [-1..1]
		float lx, ly;
		// relative right axis [-1..1]
		float rx, ry;
		// absolute left axis [0..width,0..height]
		float lxa, lya;
		// absolute right axis [0..width,0..height]
		float rxa, rya;
		// left and right triggers
		float lt, rt;
		// Flag indicating that this is a digital controller
		const bool digital;

		InputDevice(const Window& window, const Settings& settings, bool digital);
		virtual ~InputDevice(void) { }
		virtual void update(void) = 0;
		bool is_pressed(VirtualButton button) const { return buttons[static_cast<int>(button)] > 0; }
		// Returns a unique ID for this device.
		virtual int id(void) const = 0;
		const std::string& get_name(void) const { return name; }
		// Returns human-readable name for a button.
		virtual std::string get_button_name(VirtualButton button) const = 0;
		int get_mapping(VirtualButton button) const { return mapping[button]; }
		void on_press(VirtualButton button, std::function<void(void)> handler) { button_handlers[button] = handler; }
		void unbind(VirtualButton button) { button_handlers[button] = nullptr; }
		void bind_long_press(VirtualButton button, std::function<void(void)> handler) { long_press_handlers[button] = handler; }
		void unbind_long_press(VirtualButton button) { long_press_handlers[button] = nullptr; }
		InputState get_state(void) const { return InputState{ buttons, lx, ly, rx, ry, lt, rt }; }
	};
}