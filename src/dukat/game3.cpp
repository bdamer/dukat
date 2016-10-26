#include "stdafx.h"
#include "game3.h"
#include "renderer3.h"
#include "mesh.h"
#include "textmeshinstance.h"
#include "textmeshbuilder.h"

namespace dukat
{
	Game3::Game3(const Settings& settings) : GameBase(settings)
	{
		renderer = std::make_unique<dukat::Renderer3>(window.get(), shader_cache.get(), texture_cache.get());
	}

	std::unique_ptr<TextMeshInstance> Game3::create_text_mesh(float size, const Color& color)
	{
		auto mesh_instance = std::make_unique<TextMeshInstance>();
		mesh_instance->transform.update();
		mesh_instance->set_texture(texture_cache->get("font_256.png"));
		mesh_instance->set_program(shader_cache->get_program("sc_text.vsh", "sc_text.fsh"));
		TextMeshBuilder mb;
		auto mesh = mb.build_text_mesh3();
		mesh_instance->set_mesh(mesh.get());
		mesh_cache->put(mesh_cache->generate_id(), std::move(mesh));
		mesh_instance->set_size(size);
		mesh_instance->set_color(color);
		return mesh_instance;
	}

	void Game3::update(float delta)
	{
		GameBase::update(delta);
		renderer->get_camera()->update(delta);
	}
}