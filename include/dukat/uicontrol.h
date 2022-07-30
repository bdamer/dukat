#pragma once

#include "aabb2.h"

namespace dukat
{
	// Abstract base class for UI control elements.
	class UIControl
	{
	protected:
		int index;
		float priority;
		bool enabled;
		bool focus; // flag indicating that this control has focus
		std::function<void(void)> trigger_func; // function to be executed when triggered
		std::function<void(int)> cycle_func; // function to be executed when cycled
		AABB2 bb;
		
	public:
		UIControl(void) : index(-1), priority(0.0f), enabled(true), focus(false) { }
		UIControl(const UIControl& rhs) = delete;
		virtual ~UIControl(void) { }
		void operator=(const UIControl& rhs) = delete;

		// Tab index
		int get_index(void) const { return index; }
		void set_index(int index) { this->index = index; }
		// Enable flag 
		bool is_enabled(void) const { return enabled; }
		void set_enabled(bool enabled) { this->enabled = enabled; }
		bool has_trigger(void) const { return trigger_func != nullptr; }
		bool can_cycle(void) const { return cycle_func != nullptr; }
		// Screen rect
		virtual const AABB2& get_bb(void) const { return bb; }
		// Screen priority
		virtual float get_priority(void) const { return priority; }

		// Event handling
		bool is_focus(void) const { return focus; }
		virtual void set_focus(bool focus) { this->focus = focus; }
		void set_trigger(std::function<void(void)> trigger_func) { this->trigger_func = trigger_func; }
		virtual void trigger(void) { if (trigger_func) trigger_func(); }
		void set_cycle(std::function<void(int)> cycle_func) { this->cycle_func = cycle_func; }
		virtual void cycle(int dir) { if (cycle_func) cycle_func(dir); }
	};
}