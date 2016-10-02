#include "stdafx.h"
#include "game2.h"
#include "settings.h"
#include "textmeshinstance.h"
#include "textmeshbuilder.h"

namespace dukat
{
	Game2::Game2(const Settings& settings) : GameBase(settings)
	{
		renderer = std::make_unique<Renderer2>(window.get(), shader_cache.get());	
	}

	std::unique_ptr<TextMeshInstance> Game2::create_text_mesh(float size, const Color& color)
	{
		auto mesh_instance = std::make_unique<TextMeshInstance>();
		mesh_instance->transform.update();
		mesh_instance->set_texture(texture_cache->get("font_256.png"));
		mesh_instance->set_program(shader_cache->get_program("sc_ui_texture.vsh", "sc_texture.fsh"));
		TextMeshBuilder mb;
		auto mesh = mb.build_text_mesh2();
		mesh_instance->set_mesh(mesh.get());
		mesh_cache->put(mesh_cache->generate_id(), std::move(mesh));
		mesh_instance->set_size(size);
		mesh_instance->set_color(color);
		return mesh_instance;
	}

	void Game2::init(void)
	{
		GameBase::init();
		// create debug layer
		auto debug_layer = renderer->create_layer("debug", 1000.0f);
		debug_layer->hide();
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