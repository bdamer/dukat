#pragma once

#include <memory>

#include "animationmanager.h"
#include "application.h"
#include "meshcache.h"
#include "textmeshinstance.h"
#include "particlemanager.h"
#include "shadercache.h"
#include "texturecache.h"
#include "timermanager.h"

namespace dukat
{
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

		virtual void init(void);
		virtual void update(float delta);
		virtual void release(void);

		bool debug;
		// Called on a timer to output debug information.
		virtual void update_debug_text(void) { }

	public:
		GameBase(Settings& settings);
		virtual ~GameBase(void) { }
		virtual void toggle_debug(void) { debug = !debug; }
		bool is_debug(void) const { return debug; }
		std::unique_ptr<TextMeshInstance> create_text_mesh(float size = 1.0f);

		ShaderCache* get_shaders(void) const { return shader_cache.get(); }
		TextureCache* get_textures(void) const { return texture_cache.get(); }
		ParticleManager* get_particles(void) const { return particle_manager.get(); }
		TimerManager* get_timers(void) const { return timer_manager.get(); }
		AnimationManager* get_animations(void) const { return anim_manager.get(); }
		MeshCache* get_meshes(void) const { return mesh_cache.get(); }
	};
}