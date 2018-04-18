#include "stdafx.h"
#include "game2.h"
#include "settings.h"

namespace dukat
{
	Game2::Game2(Settings& settings) : GameBase(settings)
	{
		renderer = std::make_unique<Renderer2>(window.get(), shader_cache.get());
	}

	Game2::~Game2(void) 
	{
		// clear scenes to allow clean deallocation
		scenes.clear();
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