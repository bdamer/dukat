#include "stdafx.h"
#include <dukat/uimanager.h>
#include <dukat/audiomanager.h>
#include <dukat/gamebase.h>

namespace dukat
{
	static constexpr auto channel = 10;

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
		if (it == controls.rend()) // if we're at the first element, roll to end of list
			it = controls.rbegin();
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

			if (select_sample != nullptr)
				game->get_audio()->play_sample(select_sample, channel);
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
		if (it == controls.end()) // if we're at the last element, roll to the start of list
			it = controls.begin();
		it = std::find_if(it, controls.end(), 
			[](UIControl* c) { return c->is_enabled() && c->get_index() >= 0; });
		if (it != controls.end())
		{
			if (focus != nullptr)
				focus->set_focus(false);
			focus = *it;
			focus->set_focus(true);

			if (select_sample != nullptr)
				game->get_audio()->play_sample(select_sample, channel);
		}
	}

	void UIManager::focus_on(UIControl* control)
	{
		if (controls.empty() || focus == control)
			return;

		if (focus != nullptr)
			focus->set_focus(false);
		focus = control;
		focus->set_focus(true);
	}

	bool UIManager::trigger_focus(void)
	{
		if (focus != nullptr)
		{
			focus->trigger();
			if (trigger_sample != nullptr)
				game->get_audio()->play_sample(trigger_sample, channel);
			return true;
		}
		return false;
	}

	bool UIManager::cycle_focus(int dir)
	{
		if (focus != nullptr)
		{
			focus->cycle(dir);
			if (cycle_sample != nullptr)
				game->get_audio()->play_sample(cycle_sample, channel);
			return true;
		}
		return false;
	}
	
	void UIManager::set_select_sample(const std::string& sample)
	{
		if (sample.length() > 0)
			select_sample = game->get_samples()->get_sample(sample);
		else
			select_sample = nullptr;
	}

	void UIManager::set_trigger_sample(const std::string& sample)
	{
		if (sample.length() > 0)
			trigger_sample = game->get_samples()->get_sample(sample);
		else
			trigger_sample = nullptr;
	}

	void UIManager::set_cycle_sample(const std::string& sample)
	{
		if (sample.length() > 0)
			cycle_sample = game->get_samples()->get_sample(sample);
		else
			cycle_sample = nullptr;
	}
}