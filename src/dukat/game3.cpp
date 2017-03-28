#include "stdafx.h"
#include "game3.h"
#include "renderer3.h"
#include "mesh.h"

namespace dukat
{
	Game3::Game3(Settings& settings) : GameBase(settings)
	{
		renderer = std::make_unique<dukat::Renderer3>(window.get(), shader_cache.get(), texture_cache.get());
	}

	void Game3::update(float delta)
	{
		GameBase::update(delta);
		renderer->get_camera()->update(delta);
	}
}