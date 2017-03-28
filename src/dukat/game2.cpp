#include "stdafx.h"
#include "game2.h"
#include "settings.h"

namespace dukat
{
	Game2::Game2(Settings& settings) : GameBase(settings)
	{
		renderer = std::make_unique<Renderer2>(window.get(), shader_cache.get());	
	}

	void Game2::update(float delta)
	{
		GameBase::update(delta);
		renderer->get_camera()->update(delta);
	}

	void Game2::render(void)
	{
		renderer->render();
	}

	void Game2::toggle_debug(void)
	{
		GameBase::toggle_debug();
		auto debug_layer = renderer->get_layer("debug");
		if (debug_layer == nullptr)
			return;

		if (debug)
		{
			debug_layer->show();
		}
		else
		{
			debug_layer->hide();
		}
	}
}