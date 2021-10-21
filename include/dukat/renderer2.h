#pragma once

#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>

#ifndef OPENGL_VERSION
#include "version.h"
#endif // !OPENGL_VERSION

#include "buffers.h"
#include "camera2.h"
#include "light.h"
#include "matrix4.h"
#include "renderer.h"
#include "renderlayer2.h"
#include "renderstage2.h"
#include "shaderprogram.h"

namespace dukat
{
	// Forward declarations
	class MeshData;
	struct Sprite;

	// 2D Renderer
	// The coordinate system used assumes that the x-axis goes towards the right and the y-axis
	// towards the bottom of the screen.
	class Renderer2 : public Renderer
	{
	public:
		static constexpr auto max_particles = 2048;
		static constexpr auto max_lights = 16;
#if OPENGL_VERSION <= 30
		static constexpr auto u_cam_dimension = "u_cam_dimension";
#endif

		enum Flags
		{
			RenderFx = 1,
			RenderSprites = 2,
			RenderParticles = 4,
			RenderText = 8,
			// If set will force synchronization of OpenGL when
			// running in fullscreen mode.
			ForceSync = 16,
			// If set, will call glClear before rendering screen buffer
			ForceClear = 32,
			// If set, will gamma correct final rendered scene
			GammaCorrect = 64
		};

	private:
		std::unique_ptr<Camera2> camera;
		// Buffers for sprite and particle rendering shared by al layers.
		std::unique_ptr<VertexBuffer> sprite_buffer;
		std::unique_ptr<VertexBuffer> particle_buffer;
		// Framebuffer used to render layers. Dimension based on camera.
		std::unique_ptr<FrameBuffer> frame_buffer;
		// Framebuffer that matches final screen. Used for post fx.
		std::unique_ptr<FrameBuffer> screen_buffer;
		// Composite program to render screenbuffer to screen.
		ShaderProgram* composite_program;
		ShaderBinder composite_binder;
		// Used to compose layers into a single image.
		std::unique_ptr<MeshData> quad;
		// Ordered list of render stages
		std::list<std::unique_ptr<RenderStage2>> stages;
		// Lookup map for layers
		std::unordered_map<std::string, RenderLayer2*> layer_map;
		// array of lights
		std::array<Light2, max_lights> lights;
		// Render flags
		int render_flags;

		void initialize_sprite_buffers(void);
		void initialize_particle_buffers(void);
		void initialize_frame_buffers(void);
		void initialize_render_stages(void);
		RenderLayer2* create_layer(const std::string& id, RenderStage2* stage, float priority, float parallax);
		void render_layer(RenderLayer2& layer, FrameBuffer* target_buffer);
		void render_composite(FrameBuffer* target_buffer, ShaderProgram* comp_program, ShaderBinder comp_binder, Texture* source_tex);
		void render_screenbuffer(void);
		void resize_window(void);

	public:
		Renderer2(Window* window, ShaderCache* shader_cache);
		~Renderer2(void) { };

		// Draws the scene.
		void render(void);
		// Returns a pointer to a specific render stage.
		RenderStage2* get_stage(RenderStage id) const;
		// Creates a new composite with a given priority. If a render target is requested, layer will
		// have access to a dedicated texture to render to, before compositing to screen buffer.
		RenderLayer2* create_composite_layer(const std::string& id, float priority, float parallax = 1.0f, bool has_render_target = false);
		// Creates a new layer that will render directly to screen.
		RenderLayer2* create_direct_layer(const std::string& id, float priority);
		// Destroys an existing render layer.
		void destroy_layer(const std::string& id);
		// Destroys all rendering layers.
		void destroy_layers(void);
		// Returns a pointer to a named layer.
		RenderLayer2* get_layer(const std::string& id) const;
		// Adds sprite to layer.
		void add_to_layer(const std::string& id, Sprite* sprite);
		// Removs sprite from layer
		void remove_from_layer(const std::string& id, Sprite* sprite);
		// Assigns the current camera.
		void set_camera(std::unique_ptr<Camera2> camera);
		// Returns the current camera.
		Camera2* get_camera(void) const { return camera.get(); }
		// Updates uniform buffers for camera and lighting.
		void update_uniforms(void);
		// Retrieves light by index	
		Light2* get_light(int idx) { assert((idx >= 0) && (idx < max_lights)); return &lights[idx]; }
		// Updates program used to composite final image to screen.
		void set_composite_program(ShaderProgram* composite_program, std::function<void(ShaderProgram*)> composite_binder = nullptr);
		
		// Gets / sets flags
		void set_render_particles(bool val) { set_flag(render_flags, RenderParticles, val); }
		bool is_render_particles(void) const { return check_flag(render_flags, RenderParticles); }
		void set_render_effects(bool val) { set_flag(render_flags, RenderFx, val); }
		bool is_render_effects(void) const { return check_flag(render_flags, RenderFx); }
		void set_render_sprites(bool val) { set_flag(render_flags, RenderSprites, val); }
		bool is_render_sprites(void) const { return check_flag(render_flags, RenderSprites); }
		void set_render_text(bool val) { set_flag(render_flags, RenderText, val); }
		bool is_render_text(void) const { return check_flag(render_flags, RenderText); }
		void set_force_sync(bool val) { set_flag(render_flags, ForceSync, val); }
		bool is_force_sync(void) { return check_flag(render_flags, ForceSync); }
		void set_force_clear(bool val) { set_flag(render_flags, ForceClear, val); }
		bool is_force_clear(void) { return check_flag(render_flags, ForceClear); }
		void set_gamma_correct(bool val) { set_flag(render_flags, GammaCorrect, val); }
		bool is_gamma_correct(void) { return check_flag(render_flags, GammaCorrect); }
	};
}