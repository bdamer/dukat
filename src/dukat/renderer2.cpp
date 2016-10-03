#include "stdafx.h"
#include "camera2.h"
#include "renderer2.h"
#include "renderlayer2.h"
#include "shadercache.h"
#include "sprite.h"
#include "vertextypes2.h"

namespace dukat
{
	Renderer2::Renderer2(Window* window, ShaderCache* shader_cache) : Renderer(window, shader_cache)
	{ 
		// Enable transparency
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Disable depth test - we'll have to take care of rendering order ourselves
		glDisable(GL_DEPTH_TEST);

		// Enable gl_PointSize instruction in shader
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

		initialize_sprite_buffers();
		initialize_particle_buffers();

		light.position = Vector3(0.0f, 0.0f, 0.0f);
	}

	void Renderer2::initialize_sprite_buffers(void)
	{
		// Create VAO for sprite rendering
		sprite_buffer = std::make_unique<VertexBuffer>(1);
		glBindVertexArray(sprite_buffer->vao);

		// Allocate memory for vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, sprite_buffer->buffers[0]);

		// Vertex order:
		//	0 2	  3
		//	1	4 5
		TexturedVertex2 vertices[] = {
			{ -0.5f, -0.5f, 0.0f, 0.0f },
			{ -0.5f,  0.5f, 0.0f, 1.0f },
			{  0.5f, -0.5f, 1.0f, 0.0f },
			{  0.5f,  0.5f, 1.0f, 1.0f }
		};

		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(TexturedVertex2), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void Renderer2::initialize_particle_buffers(void)
	{
		// Create buffers for particle rendering
		particle_buffer = std::make_unique<VertexBuffer>(1);
		glBindVertexArray(particle_buffer->vao);

		glBindBuffer(GL_ARRAY_BUFFER, particle_buffer->buffers[0]);
		glBufferData(GL_ARRAY_BUFFER, max_particles * sizeof(ParticleVertex2), NULL, GL_STREAM_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	RenderLayer2* Renderer2::create_layer(const std::string& id, float priority, float parallax)
	{
		auto layer = std::make_unique<RenderLayer2>(shader_cache, 
			sprite_buffer.get(), particle_buffer.get(), id, priority, parallax);
		auto res = layer.get();
		bool inserted = false;
		// find position to insert based on priority
		for (auto it = layers.begin(); it != layers.end(); ++it)
		{
			if ((*it)->priority > layer->priority)
			{
				layers.insert(it, std::move(layer));
				inserted = true;
				break;
			}
		}
		// insert at the end
		if (!inserted)
		{
			layers.push_back(std::move(layer));
		}
		return res;
	}

	RenderLayer2* Renderer2::get_layer(const std::string& id) const
	{
		for (auto it = layers.begin(); it != layers.end(); ++it)
		{
			if ((*it)->id == id)
			{
				return it->get();
			}
		}
		return nullptr;
	}

	void Renderer2::add_to_layer(const std::string& id, Sprite* sprite)
	{
		get_layer(id)->add(sprite);
	}

	void Renderer2::remove_from_layer(const std::string& id, Sprite* sprite)
	{
		get_layer(id)->remove(sprite);
	}

	void Renderer2::render(void)
	{
		window->clear();

		// This will pick up the change we made to camera->transform
		update_uniforms();

		for (auto& layer : layers)
		{
			if (layer->visible())
			{
				layer->render(this);
			}
		}

		window->present();
	}

	void Renderer2::update_uniforms(void)
	{
		// Update uniform buffers
		glBindBufferBase(GL_UNIFORM_BUFFER, UniformBuffer::CAMERA, uniform_buffers->buffers[0]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraTransform2), &camera->transform, GL_STREAM_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, UniformBuffer::LIGHT, uniform_buffers->buffers[1]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(Light), &light, GL_STREAM_DRAW);
	}
}
