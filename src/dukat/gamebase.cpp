#include "stdafx.h"
#include "gamebase.h"
#include "controller.h"
#include "devicemanager.h"
#include "keyboarddevice.h"
#include "log.h"
#include "manager.h"
#include "meshcache.h"
#include "particlemanager.h"
#include "scene.h"
#include "settings.h"
#include "shadercache.h"
#include "textmeshinstance.h"
#include "textmeshbuilder.h"
#include "texturecache.h"
#include "timermanager.h"
#include "window.h"

namespace dukat
{
	GameBase::GameBase(Settings& settings) : Application(settings), controller(nullptr), debug(false)
	{
		audio_cache = std::make_unique<AudioCache>(settings.get_string("resources.samples"), settings.get_string("resources.music"));
		shader_cache = std::make_unique<ShaderCache>(settings.get_string("resources.shaders"));
		texture_cache = std::make_unique<TextureCache>(settings.get_string("resources.textures"));
		mesh_cache = std::make_unique<MeshCache>();
		add_manager<ParticleManager>();
		add_manager<TimerManager>();
		add_manager<AnimationManager>();
		add_manager<UIManager>();
		// TODO: need to rebind when devices change
		device_manager->active->on_press(InputDevice::VirtualButton::Pause, std::bind(&GameBase::toggle_pause, this));
		device_manager->active->on_press(InputDevice::VirtualButton::Debug1, std::bind(&GameBase::toggle_debug, this));
		get<TimerManager>()->create_timer(1.0f, std::bind(&GameBase::update_debug_text, this), true);
	}

	GameBase::~GameBase(void)
	{
		device_manager->active->unbind(InputDevice::VirtualButton::Pause);
		device_manager->active->unbind(InputDevice::VirtualButton::Debug1);
	}

	void GameBase::handle_event(const SDL_Event& e)
	{
		Application::handle_event(e);
		if (controller != nullptr)
			controller->handle_event(e);
	}

	void GameBase::handle_keyboard(const SDL_Event& e)
	{
		Application::handle_keyboard(e);
		if (controller != nullptr)
			controller->handle_keyboard(e);
	}

	void GameBase::update(float delta)
	{
		// Scene first so that managers can operate on updated properties.
		scene_stack.top()->update(delta);
		for (auto& it : managers)
		{
			(it.second)->update(delta);
		}
	}

	void GameBase::render(void)
	{
		scene_stack.top()->render();
	}

	void GameBase::toggle_debug(void)
	{
		debug = !debug;
		trigger(Message{Events::ToggleDebug});
	}

	std::unique_ptr<TextMeshInstance> GameBase::create_text_mesh(float size)
	{
		TextMeshBuilder mb;
		auto mesh_instance = std::make_unique<TextMeshInstance>(mb.build_text_mesh());
		mesh_instance->transform.update();
		mesh_instance->set_texture(texture_cache->get("font_256.png", ProfileMipMapped));
		mesh_instance->set_program(shader_cache->get_program("sc_text.vsh", "sc_text.fsh"));
		mesh_instance->set_size(size);
		return mesh_instance;
	}

	void GameBase::add_scene(const std::string& id, std::unique_ptr<Scene> scene)
	{
		scenes[id] = std::move(scene);
	}

	void GameBase::push_scene(const std::string& id)
	{
		if (scenes.count(id) == 0)
		{
			logger << "Failed to push scene: " << id << std::endl;
			return;
		}

		if (!scene_stack.empty())
		{
			scene_stack.top()->deactivate();
		}
		auto scene = scenes.at(id).get();
		scene_stack.push(scene);
		scene->activate();
	}

	void GameBase::pop_scene(void)
	{
		if (!scene_stack.empty())
		{
			scene_stack.top()->deactivate();
			scene_stack.pop();
			if (!scene_stack.empty())
			{
				scene_stack.top()->activate();
			}
		}
	}

	Scene* GameBase::get_scene(const std::string& id) const
	{
		if (scenes.count(id) > 0)
		{
			return scenes.at(id).get();
		}
		else
		{
			return nullptr;
		}
	}
}