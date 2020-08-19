#include "stdafx.h"
#include <dukat/game2.h>
#include <dukat/settings.h>

namespace dukat
{
	Game2::Game2(Settings& settings) : GameBase(settings)
	{
		renderer = std::make_unique<Renderer2>(window.get(), shader_cache.get());
		renderer->set_force_sync(settings.get_bool("video.forcesync", false));
		effect = std::make_unique<FullscreenEffect2>(this);
	}

	Game2::~Game2(void) 
	{
		// clear scenes to allow clean deallocation
		scenes.clear();
	}
	
	void Game2::update(float delta)
	{
		GameBase::update(delta);
		renderer->get_camera()->update(delta);
	}

	void Game2::toggle_debug(void)
	{
		GameBase::toggle_debug();
		auto debug_layer = renderer->get_layer("debug");
		if (debug_layer == nullptr)
			return;

		if (debug)
			debug_layer->show();
		else
			debug_layer->hide();
	}

	std::unique_ptr<TextMeshInstance> Game2::create_text_mesh(void)
	{
		auto tex = texture_cache->get("font_256.png", ProfileNearest);
		auto sp = shader_cache->get_program("sc_text.vsh", "sc_text.fsh");
		return build_text_mesh(tex, sp, 1.0f, -1.0f);
	}
		
	std::unique_ptr<Sprite> Game2::create_sprite(const std::string& filename, Rect rect)
	{
		Texture* texture;

		// check if this is an atlas entry
		const auto pos = filename.find_last_of('/');
		if (pos != std::string::npos)
		{
			const auto atlas_name = filename.substr(0, pos);
			const auto atlas_entry = filename.substr(pos + 1);
			auto atlas = texture_cache->get_atlas(atlas_name);
			const auto atlas_rect = atlas->get(atlas_entry);
			texture = texture_cache->get(atlas_name + ".png");

			if (rect.w == 0) // rect is empty, use atlas rect
				rect = atlas_rect;
			else // rect was specified, need to transpose by atlas position
			{
				rect.x += atlas_rect.x;
				rect.y += atlas_rect.y;
			}
		}
		else
		{
			texture = texture_cache->get(filename);
		}

		if (rect.w > 0) // rect is not empty
			return std::make_unique<Sprite>(texture, rect);
		else
			return std::make_unique<Sprite>(texture);
	}
}