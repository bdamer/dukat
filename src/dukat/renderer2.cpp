#include "stdafx.h"
#include "camera2.h"
#include "renderer2.h"
#include "renderlayer2.h"
#include "shadercache.h"
#include "sprite.h"
#include "vertextypes2.h"
#include "sysutil.h"

namespace dukat
{
	Renderer2::Renderer2(Window* window, ShaderCache* shader_cache) : Renderer(window, shader_cache)
	{ 
		// Enable transparency
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// Disable depth test - we'll have to take care of rendering order ourselves
		glDisable(GL_DEPTH_TEST);
#if OPENGL_VERSION >= 30
		// Enable gl_PointSize instruction in shader
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
#endif

		initialize_sprite_buffers();
		initialize_particle_buffers();

		light.position = Vector3(0.0f, 0.0f, 0.0f);
		gl_check_error();
	}

	void Renderer2::initialize_sprite_buffers(void)
	{
		// Vertex order:
		//	0 2 / 3
		//	1 /	4 5
		TexturedVertex2 vertices[] = {
			{ -0.5f, -0.5f, 0.0f, 0.0f },
			{ -0.5f,  0.5f, 0.0f, 1.0f },
			{ 0.5f, -0.5f, 1.0f, 0.0f },
			{ 0.5f,  0.5f, 1.0f, 1.0f }
		};
		// Create buffer for sprite rendering
		sprite_buffer = std::make_unique<VertexBuffer>(1);
		sprite_buffer->load_data(0, GL_ARRAY_BUFFER, 4, sizeof(TexturedVertex2), vertices, GL_STATIC_DRAW);
	}

	void Renderer2::initialize_particle_buffers(void)
	{
		// Create buffer for particle rendering
		particle_buffer = std::make_unique<VertexBuffer>(1);
		particle_buffer->load_data(0, GL_ARRAY_BUFFER, max_particles, sizeof(ParticleVertex2), nullptr, GL_STREAM_DRAW);
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

	void Renderer2::destroy_layer(const std::string& id)
	{
		auto it = std::find_if(layers.begin(), layers.end(), [&id](const std::unique_ptr<RenderLayer2>& layer) {
			return layer->id == id;
		});
		if (it != layers.end())
		{
			layers.erase(it);
		}
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

#if OPENGL_VERSION >= 30
		// This will pick up the change we made to camera transform. Since we're 
		// using uniform buffers, this will only be done once each frame.
		update_uniforms();
#endif

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
#if OPENGL_VERSION >= 30
		// Update uniform buffers
		glBindBufferBase(GL_UNIFORM_BUFFER, UniformBuffer::CAMERA, uniform_buffers->buffers[0]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraTransform2), &camera->transform, GL_STREAM_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, UniformBuffer::LIGHT, uniform_buffers->buffers[1]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(Light), &light, GL_STREAM_DRAW);
#else
		// Update uniform variables
		glUniformMatrix4fv(active_program->attr(Renderer2::u_cam_proj_orth), 1, false, camera->transform.mat_proj_orth.m);
		glUniformMatrix4fv(active_program->attr(Renderer2::u_cam_view), 1, false, camera->transform.mat_view.m);
		glUniform2fv(active_program->attr(Renderer2::u_cam_position), 1, (GLfloat*)(&camera->transform.position));
		glUniform2fv(active_program->attr(Renderer2::u_cam_dimension), 1, (GLfloat*)(&camera->transform.dimension));
#endif
	}
}
