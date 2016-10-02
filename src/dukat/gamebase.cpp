#include "stdafx.h"
#include "gamebase.h"
#include "devicemanager.h"
#include "keyboarddevice.h"
#include "meshcache.h"
#include "mesh.h"
#include "particlemanager.h"
#include "settings.h"
#include "shadercache.h"
#include "texturecache.h"
#include "timermanager.h"
#include "window.h"

namespace dukat
{
	GameBase::GameBase(const Settings& settings) : Application(settings)
	{
		shader_cache = std::make_unique<ShaderCache>(settings.get_string("resources.shaders"));
		texture_cache = std::make_unique<TextureCache>(settings.get_string("resources.textures"));
		particle_manager = std::make_unique<ParticleManager>();
		timer_manager = std::make_unique<TimerManager>();
		mesh_cache = std::make_unique<dukat::MeshCache>();
	}

	void GameBase::init(void)
	{
		// TODO: need to rebind when devices change
		device_manager->active->on_press(InputDevice::VirtualButton::Pause, std::bind(&GameBase::toggle_pause, this));
		device_manager->active->on_press(InputDevice::VirtualButton::Debug1, std::bind(&GameBase::toggle_debug, this));
		timer_manager->create_timer(1.0f, std::bind(&GameBase::update_debug_text, this), true);
	}

	void GameBase::update(float delta)
	{
		timer_manager->update(delta);
		particle_manager->update(delta);
	}

	void GameBase::release(void)
	{
		device_manager->active->unbind(InputDevice::VirtualButton::Pause);
		device_manager->active->unbind(InputDevice::VirtualButton::Debug1);
	}
}