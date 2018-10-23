#include "stdafx.h"
#include <dukat/game3.h>
#include <dukat/renderer3.h>
#include <dukat/settings.h>

namespace dukat
{
	Game3::Game3(Settings& settings) : GameBase(settings)
	{
		renderer = std::make_unique<dukat::Renderer3>(window.get(), shader_cache.get(), texture_cache.get());
		if (settings.get_bool("rendere.effects.enabled")) 
		{
			renderer->enable_effects();
		}

		debug_meshes.stage = RenderStage::OVERLAY;
		debug_meshes.visible = debug;

		auto debug_text = create_text_mesh();
		debug_text->set_size(1.0f / 30.0f);
		debug_text->align = TextMeshInstance::Align::Center;
		debug_text->transform.position.y = 0.75f;
		debug_text->transform.update();
		debug_meshes.add_instance(std::move(debug_text));
	}

	void Game3::update(float delta)
	{
		GameBase::update(delta);
		debug_meshes.update(delta);
		renderer->get_camera()->update(delta);
	}

	void Game3::toggle_debug(void)
	{
		GameBase::toggle_debug();
		debug_meshes.visible = debug;
	}

	void Game3::update_debug_text(void)
	{
		std::stringstream ss;
		ss << "<#yellow>"
			<< "WIN: " << window->get_width() << "x" << window->get_height()
			<< " FPS: " << get_fps()
			<< " MSH: " << dukat::perfc.avg(dukat::PerformanceCounter::MESHES)
			<< " VER: " << dukat::perfc.avg(dukat::PerformanceCounter::VERTICES) 
			<< " PAR: " << dukat::perfc.avg(dukat::PerformanceCounter::PARTICLES)
			<< " TEX: " << dukat::perfc.avg(dukat::PerformanceCounter::TEXTURES)
			<< " SHA: " << dukat::perfc.avg(dukat::PerformanceCounter::SHADERS)
			<< " FBR: " << dukat::perfc.avg(dukat::PerformanceCounter::FRAME_BUFFERS)
			<< "</>" << std::endl;
		auto debug_text = dynamic_cast<TextMeshInstance*>(debug_meshes.get_instance(0));
		debug_text->set_text(ss.str());
	}

	std::unique_ptr<TextMeshInstance> Game3::create_text_mesh(void)
	{
		auto tex = texture_cache->get("font_256.png", ProfileMipMapped);
		auto sp = shader_cache->get_program("sc_text.vsh", "sc_text.fsh");
		return build_text_mesh(tex, sp, 1.0f);
	}
}