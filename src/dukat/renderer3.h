#pragma once

#include <list>
#include <memory>
#include <array>
#include "camera3.h"
#include "effect3.h"
#include "light.h"
#include "renderer.h"
#include "texturecache.h"
#include "buffers.h"

namespace dukat
{
	// forward declarations
	class Scene;
	class Mesh;
	class MeshGroup;
	class ShaderProgram;
	class Renderable;

	class Renderer3 : public Renderer
	{
	public:
		// Size of FBO used for effects
		static constexpr const int fbo_size = 256;
		// Lighting constants
		static constexpr const int dir_light_idx = 0;
		static constexpr const int point_light_idx = 1;
		static constexpr const int max_point_lights = 4;
		static constexpr const int num_lights = max_point_lights + 1;

#if OPENGL_VERSION <= 30
		static constexpr const char* u_cam_dir = "u_cam_dir";
		static constexpr const char* u_cam_up = "u_cam_up";
		static constexpr const char* u_cam_left = "u_cam_left";
#endif

	private:
		std::unique_ptr<Camera3> camera;
		// By convention, the 1st light is a directional light. All other lights are positional lights.
		std::array<Light, num_lights> lights;

		// Framebuffer effects
		bool effects_enabled;
		std::list<Effect3> effects;
		// fullscreen framebuffer
		std::unique_ptr<FrameBuffer> fb0;
		// effect framebuffer
		std::unique_ptr<FrameBuffer> fb1;
		std::unique_ptr<FrameBuffer> fb2;
		// used to chain effects
		FrameBuffer* frame_buffer;
		// Quad to composite final image onto
		std::unique_ptr<Mesh> quad;
		ShaderProgram* composite_program;
		
		void init_lights(void);
		void switch_fbo(void);

	public:
		Renderer3(Window* window, ShaderCache* shaders, TextureCache* textures);
		~Renderer3(void) { }

		void render(const std::vector<Renderable*>& meshes);
		// Updates uniform buffers for camera and lighting.
		void update_uniforms(void);

		// Framebuffer effects
		void enable_effects(void) { effects_enabled = true; }
		void disable_effects(void) { effects_enabled = false; }
		void toggle_effects(void) { effects_enabled = !effects_enabled; }
		void add_effect(int index, const Effect3& effect);

		void set_camera(std::unique_ptr<Camera3> camera) { this->camera = std::move(camera); }
		Camera3* get_camera(void) const { return camera.get(); }
		Light* get_light(int idx) { assert((idx >= 0) && (idx < num_lights)); return &lights[idx]; }
	};
}