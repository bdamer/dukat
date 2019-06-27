#pragma once

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
	private:
		std::unique_ptr<Camera2> camera;
		std::unique_ptr<VertexBuffer> sprite_buffer;
		std::unique_ptr<VertexBuffer> particle_buffer;
		std::unique_ptr<FrameBuffer> frame_buffer;
		// Used to compose the layers into a single image.
		std::unique_ptr<MeshData> quad;
		// list of layers ordered by priority
		std::vector<std::unique_ptr<RenderLayer2>> layers;
		// just a single light for now
		Light light;
		// Render flags
		bool render_effects;
		bool render_sprites;
		bool render_particles;
		bool render_text;

		void initialize_sprite_buffers(void);
		void initialize_particle_buffers(void);
		void initialize_frame_buffer(void);
		void render_layer(RenderLayer2& layer);

	public:
		static const int max_particles = 2048;

#if OPENGL_VERSION <= 30
		static constexpr const char* u_cam_dimension = "u_cam_dimension";
#endif
		Renderer2(Window* window, ShaderCache* shader_cache);
		~Renderer2(void) { };

		// Draws the scene.
		void render(void);
		// Creates a new layer with a given priority.
		RenderLayer2* create_layer(const std::string& id, float priority, float parallax = 1.0f, bool has_render_target = false);
		// Helper for creating overlay layers
		RenderLayer2* create_overlay_layer(const std::string& id, float priority);
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
		void set_light(const Light& light) { this->light = light; }
		// Gets / sets flags
		void set_render_particles(bool val) { render_particles = val; }
		bool is_render_particles(void) const { return render_particles; }
		void set_render_effects(bool val) { render_effects = val; }
		bool is_render_effects(void) const { return render_effects; }
		void set_render_sprites(bool val) { render_sprites = val; }
		bool is_render_sprites(void) const { return render_sprites; }
		void set_render_text(bool val) { render_text = val; }
		bool is_render_text(void) const { return render_text; }
	};
}