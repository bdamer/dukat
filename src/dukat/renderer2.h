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
		// list of layers ordered by priority
		std::vector<std::unique_ptr<RenderLayer2>> layers;
		// just a single light for now
		Light light;

		void initialize_sprite_buffers(void);
		void initialize_particle_buffers(void);

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
		RenderLayer2* create_layer(const std::string& id, float priority, float parallax = 1.0f);
		// Destroys an existing render layer.
		void destroy_layer(const std::string& id);
		// Returns a pointer to a named layer.
		RenderLayer2* get_layer(const std::string& id) const;
		// Adds sprite to layer.
		void add_to_layer(const std::string& id, Sprite* sprite);
		// Removs sprite from layer
		void remove_from_layer(const std::string& id, Sprite* sprite);
		// Assigns the current camera.
		void set_camera(std::unique_ptr<Camera2> camera) { this->camera = std::move(camera); }
		// Returns the current camera.
		Camera2* get_camera(void) const { return camera.get(); }
		// Updates uniform buffers for camera and lighting.
		void update_uniforms(void);
		void set_light(const Light& light) { this->light = light; }
	};
}