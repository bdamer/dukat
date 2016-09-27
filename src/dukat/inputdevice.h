#pragma once

#include <functional>
#include <map>

namespace dukat
{
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

		// Describes how to normalize input axis
		enum AxisNormalization
		{
			Dependent, // normalize based on radius (x and y are dependent
			Independent
		};

	private:
		static std::map<VirtualButton, std::function<void(void)>> handlers;
		bool buttons[VirtualButton::_Count];

	protected:
		int mapping[VirtualButton::_Count];
		void udapte_button_state(VirtualButton button, bool pressed);

	public:
		const int id;
		// left axis [-1..1]
		float lx, ly;
		// right axis [-1..1]
		float rx, ry;
		// left and right triggers
		float lt, rt;
		// Flag indicating that this is a digital controller
		const bool digital;
		// TODO: not supported by all controllers
		AxisNormalization normalization;

		InputDevice(int id, bool digital) : lx(0.0f), ly(0.0f), rx(0.0f), ry(0.0f), lt(0.0f), rt(0.0f), id(id), digital(digital) { }
		virtual ~InputDevice(void) { }
		virtual void update(void) = 0;
		virtual bool is_pressed(VirtualButton button) const = 0;
		void on_press(VirtualButton button, std::function<void(void)> handler) { handlers[button] = handler; }
		void unbind(VirtualButton button) { handlers.erase(button); }
	};
}