#include "stdafx.h"
#include "gamebase.h"
#include "devicemanager.h"
#include "keyboarddevice.h"
#include "meshcache.h"
#include "mesh.h"
#include "particlemanager.h"
#include "settings.h"
#include "shadercache.h"
#include "textmeshinstance.h"
#include "textmeshbuilder.h"
#include "texturecache.h"
#include "timermanager.h"
#include "window.h"

namespace dukat
{
	GameBase::GameBase(Settings& settings) : Application(settings), debug(false)
	{
		shader_cache = std::make_unique<ShaderCache>(settings.get_string("resources.shaders"));
		texture_cache = std::make_unique<TextureCache>(settings.get_string("resources.textures"));
		particle_manager = std::make_unique<ParticleManager>();
		timer_manager = std::make_unique<TimerManager>();
		anim_manager = std::make_unique<AnimationManager>();
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
		anim_manager->update(delta);
		particle_manager->update(delta);
	}

	void GameBase::release(void)
	{
		device_manager->active->unbind(InputDevice::VirtualButton::Pause);
		device_manager->active->unbind(InputDevice::VirtualButton::Debug1);
	}

	std::unique_ptr<TextMeshInstance> GameBase::create_text_mesh(float size)
	{
		TextMeshBuilder mb;
		auto mesh_instance = std::make_unique<TextMeshInstance>(mb.build_text_mesh());
		mesh_instance->transform.update();
		mesh_instance->set_texture(texture_cache->get("font_256.png"));
		mesh_instance->set_program(shader_cache->get_program("sc_text.vsh", "sc_text.fsh"));
		mesh_instance->set_size(size);
		return mesh_instance;
	}
}