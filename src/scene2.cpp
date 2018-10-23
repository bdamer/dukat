#include "stdafx.h"
#include "scene2.h"
#include "game2.h"

namespace dukat
{
	void Scene2::update(float delta)
	{
		auto renderer = game->get_renderer();
		renderer->get_camera()->update(delta);
	}

	void Scene2::render(void)
	{
		auto renderer = game->get_renderer();
		renderer->render();
	}
}