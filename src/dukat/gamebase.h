#pragma once

#include <memory>
#include <stack>

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

	// Abstract base class for game implementations.
	class GameBase : public Application
	{
	protected:
		std::unique_ptr<ShaderCache> shader_cache;
		std::unique_ptr<TextureCache> texture_cache;
		std::unique_ptr<MeshCache> mesh_cache;
		std::unique_ptr<ParticleManager> particle_manager;
		std::unique_ptr<TimerManager> timer_manager;
		std::unique_ptr<AnimationManager> anim_manager;
		std::unique_ptr<UIManager> ui_manager;
		std::unordered_map<std::string, std::unique_ptr<Scene>> scenes;
		std::stack<Scene*> scene_stack;
		Controller* controller;

		virtual void handle_event(const SDL_Event& e);
		virtual void handle_keyboard(const SDL_Event& e);
		virtual void update(float delta);
		virtual void render(void);
		virtual void receive(const Message& message) { }

		bool debug;
		// Called on a timer to output debug information.
		virtual void update_debug_text(void) { }

	public:
		GameBase(Settings& settings);
		virtual ~GameBase(void);
		virtual void toggle_debug(void);
		bool is_debug(void) const { return debug; }
		std::unique_ptr<TextMeshInstance> create_text_mesh(float size = 1.0f);

		void add_scene(const std::string& id, std::unique_ptr<Scene> scene);
		void push_scene(const std::string& id);
		void pop_scene(void);
		Scene* get_scene(const std::string& id) const;
		void set_controller(Controller* controller) { this->controller = controller; }

		ShaderCache* get_shaders(void) const { return shader_cache.get(); }
		TextureCache* get_textures(void) const { return texture_cache.get(); }
		ParticleManager* get_particles(void) const { return particle_manager.get(); }
		TimerManager* get_timers(void) const { return timer_manager.get(); }
		AnimationManager* get_animations(void) const { return anim_manager.get(); }
		UIManager* get_ui(void) const { return ui_manager.get(); }
		MeshCache* get_meshes(void) const { return mesh_cache.get(); }
		Scene* get_scene(void) const { return scene_stack.top(); }
	};
}