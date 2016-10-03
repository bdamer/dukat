#pragma once

#include <memory>
#include "camera3.h"
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

	enum RenderStage
	{
		SCENE,
		OVERLAY
	};

	class Renderer3 : public Renderer
	{
	private:
		// Size of FBO used for effects
		static const int fbo_size = 256;

		bool use_fbo;

		std::unique_ptr<Camera3> camera;

		// Testing
		ShaderProgram* composite_program;
		ShaderProgram* threshold_program;
		ShaderProgram* blur_program;

		// fullscreen framebuffer
		std::unique_ptr<FrameBuffer> fb0;
		// effect framebuffer
		std::unique_ptr<FrameBuffer> fb1;
		std::unique_ptr<FrameBuffer> fb2;
		// used to chain effects
		FrameBuffer* frame_buffer;
		// Quad to composite final image onto
		std::unique_ptr<Mesh> quad;

		Light light;

		void switch_fbo(void);

	public:
		Renderer3(Window* window, ShaderCache* shaders, TextureCache* textures);
		~Renderer3(void) { }

		void render(const std::vector<MeshGroup*>& meshes);
		// Updates uniform buffers for camera and lighting.
		void update_uniforms(void);

		// Framebuffer effects
		void enable_fbo(void) { use_fbo = true; }
		void disable_fbo(void) { use_fbo = false; }
		void toggle_fbo(void) { use_fbo = !use_fbo; }

		void set_camera(std::unique_ptr<Camera3> camera) { this->camera = std::move(camera); }
		Camera3* get_camera(void) const { return camera.get(); }
        // TODO: handle multiple light sources
        Light& get_light(int idx) { return light; }
	};
}