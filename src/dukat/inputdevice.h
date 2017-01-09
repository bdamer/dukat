#pragma once

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
			PrimaryAction,
            SecondaryAction,
			Debug1,
			Debug2,
			Debug3,
			Debug4,
			Pause,
			_Count
		};

	private:
		static std::map<VirtualButton, std::function<void(void)>> handlers;
		bool buttons[VirtualButton::_Count];

	protected:
		Window* window;
		int mapping[VirtualButton::_Count];
		void update_button_state(VirtualButton button, bool pressed);

	public:
		const int id;
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

		InputDevice(Window* window, int id, bool digital) : window(window), id(id), digital(digital),
			lx(0.0f), ly(0.0f), rx(0.0f), ry(0.0f), lxa(0.0f), lya(0.0f), rxa(0.0f), rya(0.0f), lt(0.0f), rt(0.0f) { }
		virtual ~InputDevice(void) { }
		virtual void update(void) = 0;
		virtual bool is_pressed(VirtualButton button) const = 0;
		void on_press(VirtualButton button, std::function<void(void)> handler) { handlers[button] = handler; }
		void unbind(VirtualButton button) { handlers.erase(button); }
	};
}