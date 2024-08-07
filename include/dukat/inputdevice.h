#pragma once

#include <array>
#include <functional>
#include <map>
#include "inputstate.h"

namespace dukat
{
	class Window;
	class Settings;

	struct LongPressHandler
	{
		enum Event
		{
			Begin,
			Complete,
			Cancel
		};

		Uint32 threshold;
		std::function<void(Event ev)> callback;

		LongPressHandler(void) : threshold(0), callback(nullptr) { }
		LongPressHandler(nullptr_t) : threshold(0), callback(nullptr) { }
		LongPressHandler(Uint32 threshold, const std::function<void(Event)>& callback)
			: threshold(threshold), callback(callback) { }
	};

	inline bool operator==(const LongPressHandler& handler, nullptr_t) { return handler.callback == nullptr; }
	inline bool operator!=(const LongPressHandler& handler, nullptr_t) { return handler.callback != nullptr; }

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
		static std::array<LongPressHandler, VirtualButton::_Count> long_press_handlers;
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

		// Returns a unique ID for this device.
		virtual int id(void) const = 0;
		const std::string& get_name(void) const { return name; }

		// Returns human-readable name for a button.
		virtual std::string get_button_name(VirtualButton button) const = 0;
		int get_mapping(VirtualButton button) const { return mapping[button]; }
		bool is_pressed(VirtualButton button) const { return buttons[static_cast<int>(button)] > 0; }
		// Temporarily overrides key mapping.
		void override_mapping(VirtualButton button, int key) { mapping[button] = key; }
		// Restores default mapping based on configration settings.
		virtual void restore_mapping(const Settings& settings) { };

		// Event handlers
		void on_press(VirtualButton button, std::function<void(void)> handler) { button_handlers[button] = handler; }
		void unbind(VirtualButton button) { button_handlers[button] = nullptr; }

		// Binds event handler for long-press.
		void bind_long_press(VirtualButton button, const std::function<void(LongPressHandler::Event)>& handler, Uint32 threshold = 0) {
			long_press_handlers[button] = LongPressHandler(threshold > 0 ? threshold : long_press_threshold, handler);
		}
		void unbind_long_press(VirtualButton button) { long_press_handlers[button] = nullptr; }

		// Controls haptic feedback
		virtual void start_feedback(float low_freq, float hi_freq, float duration) { }
		virtual void cancel_feedback(void) { }

		// Returns input state descriptor for replay
		InputState get_state(void) const { return InputState{ buttons, lx, ly, rx, ry, lt, rt }; }
	};
}