#include "stdafx.h"
#include <dukat/scene2.h>
#include <dukat/game2.h>

namespace dukat
{
	void Scene2::update(float delta)
	{
		// Execute any delayed actions once
		while (!delayed_actions.empty())
		{
			const auto& action = delayed_actions.front();
			action();
			delayed_actions.pop();
		}
	}

	void Scene2::render(void)
	{
		auto renderer = game->get_renderer();
		renderer->render();
	}
}