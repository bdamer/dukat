#pragma once

#include "aabb2.h"

namespace dukat
{
	// Abstract base class for UI control elements.
	class UIControl
	{
	protected:
		int index;
		AABB2 bb;
		float priority;
		bool enabled;

	public:
		UIControl(void) : index(-1), priority(0.0f), enabled(true) { }
		UIControl(const UIControl& rhs) = delete;
		virtual ~UIControl(void) { }
		void operator=(const UIControl& rhs) = delete;

		// Tab index
		int get_index(void) { return index; }
		void set_index(int index) { this->index = index; }
		// Enable flag 
		bool is_enabled(void) { return enabled; }
		void set_enabled(bool enabled) { this->enabled = enabled; }
		// Screen rect
		virtual const AABB2& get_bb(void) { return bb; }
		// Screen priority
		virtual float get_priority(void) { return priority; }

		// Event handlers
		virtual void gain_focus(void) { }
		virtual void lose_focus(void) { }
		virtual void trigger(void) { }
	};
}