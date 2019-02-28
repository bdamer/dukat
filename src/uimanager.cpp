#include "stdafx.h"
#include <dukat/uimanager.h>

namespace dukat
{
	void UIManager::add_control(UIControl* control)
	{
		for (auto it = controls.begin(); it != controls.end(); ++it)
		{
			if ((*it)->get_priority() < control->get_priority())
			{
				controls.insert(it, control);
				return;
			}
		}
		controls.insert(controls.end(), control);
	}

	void UIManager::remove_control(UIControl* control)
	{
		auto it = std::find(controls.begin(), controls.end(), control);
		if (it != controls.end())
		{
			controls.erase(it);
		}
		if (control == focus)
		{
			focus->set_focus(false);
			focus = nullptr;
		}
	}

	bool UIManager::handle_click(const Vector2 & pos)
	{
		for (auto ctrl : controls)
		{
			if (ctrl->is_enabled() && ctrl->get_bb().contains(pos))
			{
				ctrl->trigger();
				return true;
			}
		}
		return false;
	}

	void UIManager::first_control(void)
	{
		if (controls.empty())
			return;
		auto it = std::find_if(controls.begin(), controls.end(),
			[](UIControl* c) { return c->is_enabled() && c->get_index() >= 0; });
		if (it != controls.end())
		{
			if (focus != nullptr)
			{
				focus->set_focus(false);
			}
			focus = *it;
			focus->set_focus(true);
		}
	}

	// search for previous element starting from current focus
	void UIManager::prev_control(void)
	{
		if (controls.empty())
			return;
		auto it = (focus == nullptr) ? controls.rbegin() :
			std::find(controls.rbegin(), controls.rend(), focus);
		if (*it == focus)
		{
			++it; // skip current element in focus
		}
		it = std::find_if(it, controls.rend(),
			[](UIControl* c) { return c->is_enabled() && c->get_index() >= 0; });
		if (it != controls.rend())
		{
			if (focus != nullptr)
			{
				focus->set_focus(false);
			}
			focus = *it;
			focus->set_focus(true);
		}
	}

	// search for next element starting from current focus
	void UIManager::next_control(void)
	{
		if (controls.empty())
			return;
		auto it = (focus == nullptr) ? controls.begin() : 
			std::find(controls.begin(), controls.end(), focus);
		if (*it == focus)
		{
			++it; // skip current element in focus
		}
		it = std::find_if(it, controls.end(), 
			[](UIControl* c) { return c->is_enabled() && c->get_index() >= 0; });
		if (it != controls.end())
		{
			if (focus != nullptr)
			{
				focus->set_focus(false);
			}
			focus = *it;
			focus->set_focus(true);
		}
	}

	bool UIManager::trigger_focus(void)
	{
		if (focus != nullptr)
		{
			focus->trigger();
			return true;
		}
		return false;
	}

	bool UIManager::cycle_focus(int dir)
	{
		if (focus != nullptr)
		{
			focus->cycle(dir);
			return true;
		}
		return false;
	}
}