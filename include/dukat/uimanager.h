#pragma once

#include <list>

#include "manager.h"
#include "uicontrol.h"

namespace dukat
{
	class Vector2;

	class UIManager : public Manager
	{
	private:
		std::list<UIControl*> controls;
		UIControl* focus;

	public:
		UIManager(GameBase* game) : Manager(game), focus(nullptr) { }
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
		// Triggers click on control in focus, if any.
		bool trigger_focus(void); 
	};
}