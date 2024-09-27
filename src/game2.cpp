#include "stdafx.h"
#include <dukat/game2.h>
#include <dukat/log.h>
#include <dukat/settings.h>

namespace dukat
{
	Game2::Game2(Settings& settings) : GameBase(settings), speed_factor(1.0f)
	{
		const auto start = SDL_GetTicks();
		renderer = std::make_unique<Renderer2>(window.get(), shader_cache.get());
		renderer->set_force_sync(settings.get_bool("video.forcesync", false));
		log->trace("Renderer initialized in {} ms", SDL_GetTicks() - start);
		effect = std::make_unique<FullscreenEffect2>(this);
	}

	Game2::~Game2(void) 
	{
		// clear scenes to allow clean deallocation
		scenes.clear();
	}
	
	void Game2::update(float delta)
	{
		const auto scaled_delta = speed_factor * delta;
		GameBase::update(scaled_delta);
		renderer->get_camera()->update(scaled_delta);
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

	void Game2::save_screenshot(const std::string& filename)
	{
		log->info("Saving screenshot to: {}", filename);
		auto surface = renderer->copy_screen_buffer();
		save_surface(*surface, filename);
	}

	std::unique_ptr<TextMeshInstance> Game2::create_text_mesh(const std::string& font_name)
	{
		auto sp = shader_cache->get_program("sc_text.vsh", "sc_text.fsh");
		return build_text_mesh(font_name, sp, 1.0f, -1.0f);
	}
		
	std::unique_ptr<Sprite> Game2::create_sprite(const std::string& filename, Rect rect)
	{
		Texture* texture;

		// check if this is an atlas entry
		std::string atlas_name;
		Rect atlas_rect;
		try
		{
			if (texture_cache->atlas_lookup(filename, atlas_name, atlas_rect))
			{
				texture = texture_cache->get_or_load(atlas_name + ".png");
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
				texture = texture_cache->get_or_load(filename);
			}
		}
		catch (const std::runtime_error& ex)
		{
			log->warn("Failed to create sprite: {}", ex.what());
			texture = texture_cache->get_or_load("missing.png");
			rect.w = 0;
		}

		if (rect.w > 0) // rect is not empty
			return std::make_unique<Sprite>(texture, rect);
		else
			return std::make_unique<Sprite>(texture);
	}
}