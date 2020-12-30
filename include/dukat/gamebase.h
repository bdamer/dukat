#pragma once

#include <memory>
#include <queue>
#include <stack>
#include <typeindex>

#ifndef __ANDROID__
#include "audiocache.h"
#endif
#include "animationmanager.h"
#include "application.h"
#include "meshcache.h"
#include "messenger.h"
#include "textmeshinstance.h"
#include "particlemanager.h"
#include "shadercache.h"
#include "texturecache.h"
#include "timermanager.h"
#include "uimanager.h"

namespace dukat
{
	class Scene;
	class Controller;
	class Manager;

	// Abstract base class for game implementations.
	class GameBase : public Application
	{
	protected:
#ifndef __ANDROID__
		std::unique_ptr<AudioCache> audio_cache;
#endif
		std::unique_ptr<ShaderCache> shader_cache;
		std::unique_ptr<TextureCache> texture_cache;
		std::unique_ptr<MeshCache> mesh_cache;
		std::map<std::type_index, std::unique_ptr<Manager>> managers;
		std::unordered_map<std::string, std::unique_ptr<Scene>> scenes;
		std::stack<Scene*> scene_stack;
		std::queue<std::function<void(void)>> delayed_actions;
		Controller* controller;

		virtual void handle_event(const SDL_Event& e);
		virtual void handle_keyboard(const SDL_Event& e);
		virtual void update(float delta);
		virtual void render(void);
		virtual void receive(const Message& message) { }

		bool debug;
		// Called on a timer to output debug information.
		virtual void update_debug_text(void) { }

		std::unique_ptr<TextMeshInstance> build_text_mesh(Texture* texture, ShaderProgram* sp, float size, float yorientation);

	public:
		GameBase(Settings& settings);
		virtual ~GameBase(void);
		virtual void toggle_debug(void);
		bool is_debug(void) const { return debug; }
		virtual std::unique_ptr<TextMeshInstance> create_text_mesh(void) = 0;

		void add_scene(const std::string& id, std::unique_ptr<Scene> scene);
		void push_scene(const std::string& id);
		void pop_scene(void);
		Scene* get_scene(const std::string& id) const;
		Scene* get_scene(void) const { return scene_stack.top(); }
		void set_controller(Controller* controller) { this->controller = controller; }

		// Schedules an action to be executed at the start of the next frame.
		void delay_action(const std::function<void(void)>& action) { delayed_actions.push(action); }

		// Retrieves a registered manager.
		template <typename T>
		T* get(void) const;
		// Registers a new manager.
		template <typename T>
		T* add_manager(void);
		// Removes a registered manager.
		template <typename T>
		void remove_manager(void);

#ifndef __ANDROID__
		AudioCache* get_samples(void) const { return audio_cache.get(); }
#endif
		ShaderCache* get_shaders(void) const { return shader_cache.get(); }
		TextureCache* get_textures(void) const { return texture_cache.get(); }
		MeshCache* get_meshes(void) const { return mesh_cache.get(); }
		TimerManager* get_timers(void) const { return get<TimerManager>(); }
		AnimationManager* get_animations(void) const { return get<AnimationManager>(); }
	};

	// Define template methods here:
	template <typename T>
	T* GameBase::get(void) const
	{
		std::type_index index(typeid(T));
		if (managers.count(index) != 0)
		{
			return static_cast<T*>(managers.at(index).get());
		}
		else
		{
			return nullptr;
		}
	}

	template <typename T>
	T* GameBase::add_manager(void)
	{
		std::type_index index(typeid(T));
		managers[index] = std::unique_ptr<T>(new T(this));
		return static_cast<T*>(managers[index].get());
	}

	template <typename T>
	void GameBase::remove_manager(void)
	{
		std::type_index index(typeid(T));
		managers.erase(index);
	}
}