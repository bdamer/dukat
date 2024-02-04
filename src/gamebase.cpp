#include "stdafx.h"
#include <dukat/gamebase.h>
#include <dukat/controller.h>
#include <dukat/devicemanager.h>
#include <dukat/inputrecorder.h>
#include <dukat/keyboarddevice.h>
#include <dukat/log.h>
#include <dukat/manager.h>
#include <dukat/meshcache.h>
#include <dukat/particlemanager.h>
#include <dukat/scene.h>
#include <dukat/settings.h>
#include <dukat/shadercache.h>
#include <dukat/textmeshinstance.h>
#include <dukat/textmeshbuilder.h>
#include <dukat/texturecache.h>
#include <dukat/timermanager.h>
#include <dukat/window.h>

namespace dukat
{
	GameBase::GameBase(Settings& settings) : Application(settings), controller(nullptr), debug(false)
	{
#ifndef __ANDROID__
		audio_cache = std::make_unique<AudioCache>(settings.get_string("resources.samples"), settings.get_string("resources.music"));
#endif
		shader_cache = std::make_unique<ShaderCache>(settings.get_string("resources.shaders"));
		texture_cache = std::make_unique<TextureCache>(settings.get_string("resources.textures"));
		mesh_cache = std::make_unique<MeshCache>();
		font_cache = std::make_unique<FontCache>(settings.get_string("resources.fonts"));
		add_manager<ParticleManager>();
		add_manager<TimerManager>();
		add_manager<AnimationManager>();
		add_manager<UIManager>();
		device_manager->active->on_press(InputDevice::VirtualButton::Debug, std::bind(&GameBase::toggle_debug, this));
		get<TimerManager>()->create(1.0f, std::bind(&GameBase::update_debug_text, this), true);
		window->subscribe(this, events::WindowResized);
	}

	GameBase::~GameBase(void)
	{
		window->unsubscribe(this, events::WindowResized);
		device_manager->active->unbind(InputDevice::VirtualButton::Debug);

		// explicitely free resources
		scenes.clear();
		for (auto it = managers.cbegin(); it != managers.cend(); )
			it = managers.erase(it);
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
		// Execute any delayed actions once
		while (!delayed_actions.empty())
		{
			const auto& action = delayed_actions.front();
			action();
			delayed_actions.pop();
		}

		// Scene first so that managers can operate on updated properties.
		scene_stack.top()->update(delta);
		for (auto& it : managers)
		{
			if (it.second->is_enabled())
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
		trigger(Message{events::ToggleDebug});
	}

	std::unique_ptr<TextMeshInstance> GameBase::build_text_mesh(const std::string& font_name, ShaderProgram* sp, float size, float yorientation)
	{
		auto font = font_cache->get(font_name);
		auto mesh_instance = std::make_unique<TextMeshInstance>(font, yorientation);
		mesh_instance->transform.update();
		mesh_instance->set_program(sp);
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
			log->warn("Failed to push scene: {}", id);
			return;
		}

		if (!scene_stack.empty())
		{
			scene_stack.top()->deactivate();
		}
		auto scene = scenes.at(id).get();
		scene_stack.push(scene);
		scene->activate();
		// Reset last update whenever scene changes.
		last_update = SDL_GetTicks();
	}

	void GameBase::pop_scene(int n)
	{
		assert(n > 0);
		assert(!scene_stack.empty()); // invalid call

		scene_stack.top()->deactivate();

		while (n > 0 && !scene_stack.empty())
		{
			scene_stack.pop();
			n--;
		}

		// Reset last update whenever scene changes.
		last_update = SDL_GetTicks();

		if (!scene_stack.empty())
			scene_stack.top()->activate();
	}

	Scene* GameBase::get_scene(const std::string& id) const
	{
		if (scenes.count(id) > 0)
			return scenes.at(id).get();
		else
			return nullptr;
	}

	void GameBase::receive(const Message& message)
	{
		switch (message.event) {
		case events::WindowResized:
			// delay resize notification to avoid subscription inside event notification
			const auto width = *message.get_param1<int>();
			const auto height = *message.get_param2<int>();
			delay_action([&, width, height](void) {
				if (!scene_stack.empty())
					scene_stack.top()->resize(width, height);
			});
			break;
		}
	}
}