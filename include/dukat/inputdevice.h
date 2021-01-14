#pragma once

#include <array>
#include <functional>
#include <map>

namespace dukat
{
	class Window;

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
		static std::map<VirtualButton, std::function<void(void)>> handlers;
		std::array<bool, VirtualButton::_Count> buttons;

	protected:
		std::string name;
		Window* window;
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

		InputDevice(Window* window, bool digital) : window(window),	lx(0.0f), ly(0.0f), 
			rx(0.0f), ry(0.0f), lxa(0.0f), lya(0.0f), rxa(0.0f), rya(0.0f), lt(0.0f), rt(0.0f), digital(digital) { }
		virtual ~InputDevice(void) { }
		virtual void update(void) = 0;
		virtual bool is_pressed(VirtualButton button) const = 0;
		// Returns a unique ID for this device.
		virtual int id(void) const = 0;
		const std::string& get_name(void) const { return name; }
		// Returns human-readable name for a button.
		virtual std::string get_button_name(VirtualButton button) const = 0;
		int get_mapping(VirtualButton button) const { return mapping[button]; }
		void on_press(VirtualButton button, std::function<void(void)> handler) { handlers[button] = handler; }
		void unbind(VirtualButton button) { handlers.erase(button); }
	};
}