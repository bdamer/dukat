#pragma once

#include <list>

#include "manager.h"
#include "uicontrol.h"

namespace dukat
{
	struct Sample;
	class Vector2;

	class UIManager : public Manager
	{
	private:
		std::list<UIControl*> controls;
		UIControl* focus;
		Sample* select_sample;
		Sample* trigger_sample;
		Sample* cycle_sample;

	public:
		UIManager(GameBase* game) : Manager(game), focus(nullptr), select_sample(nullptr), trigger_sample(nullptr), cycle_sample(nullptr) { }
		~UIManager(void) { }

		// Adds and removes control.
		void add_control(UIControl* control);
		void remove_control(UIControl* control);
		// Relays click event to registered controls. Returns true if 
		// a control was triggered.
		bool handle_click(const Vector2& pos);
		// Changes control in focus.
		void first_control(void);
		void prev_control(void);
		void next_control(void);
		void focus_on(UIControl* control);
		const UIControl* get_focus(void) const { return focus; }
		// Triggers click on control in focus, if any.
		bool trigger_focus(void); 
		// Cycles current control in focus, if any.
		bool cycle_focus(int dir);

		// Loads audio samples for UI navigation actions
		void set_select_sample(const std::string& sample);
		void set_trigger_sample(const std::string& sample);
		void set_cycle_sample(const std::string& sample);
	};
}