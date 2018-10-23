#include "stdafx.h"
#include <dukat/game2.h>
#include <dukat/settings.h>

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

	std::unique_ptr<TextMeshInstance> Game2::create_text_mesh(void)
	{
		auto tex = texture_cache->get("font_256.png", ProfileNearest);
		auto sp = shader_cache->get_program("sc_text.vsh", "sc_text.fsh");
		return build_text_mesh(tex, sp, 1.0f);
	}
}