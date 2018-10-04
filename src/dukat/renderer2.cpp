#include "stdafx.h"
#include "camera2.h"
#include "log.h"
#include "renderer2.h"
#include "renderlayer2.h"
#include "shadercache.h"
#include "sprite.h"
#include "vertextypes2.h"
#include "sysutil.h"

#include "meshbuilder2.h"
#include "meshdata.h"
#include "textureutil.h"

namespace dukat
{
	Renderer2::Renderer2(Window* window, ShaderCache* shader_cache) : Renderer(window, shader_cache), composite_binder(nullptr), 
		render_effects(true), render_sprites(true), render_particles(true), render_text(true)
	{
		// Enable transparency
		set_blending(true);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// Disable depth test - we'll have to take care of rendering order ourselves
		glDisable(GL_DEPTH_TEST);
		// Enable gl_PointSize instruction in shader
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

		initialize_sprite_buffers();
		initialize_particle_buffers();
		initialize_frame_buffer();

		MeshBuilder2 builder;
		quad = builder.build_textured_quad();
		// Load default composite program
		composite_program = shader_cache->get_program("fx_default.vsh", "fx_default.fsh");

		light.position = Vector3(0.0f, 0.0f, 0.0f);
		gl_check_error();
	}

	void Renderer2::initialize_sprite_buffers(void)
	{
		// Vertex order:
		//	0 2 / 3
		//	1 /	4 5
		Vertex2PT vertices[] = {
			{ -0.5f, -0.5f, 0.0f, 0.0f },
			{ -0.5f,  0.5f, 0.0f, 1.0f },
			{ 0.5f, -0.5f, 1.0f, 0.0f },
			{ 0.5f,  0.5f, 1.0f, 1.0f }
		};
		// Create buffer for sprite rendering
		sprite_buffer = std::make_unique<VertexBuffer>(1);
		sprite_buffer->load_data(0, GL_ARRAY_BUFFER, 4, sizeof(Vertex2PT), vertices, GL_STATIC_DRAW);
	}

	void Renderer2::initialize_particle_buffers(void)
	{
		// Create buffer for particle rendering
		particle_buffer = std::make_unique<VertexBuffer>(1);
		particle_buffer->load_data(0, GL_ARRAY_BUFFER, max_particles, sizeof(Vertex2PSC), nullptr, GL_STREAM_DRAW);
	}

	void Renderer2::initialize_frame_buffer(void)
	{
		log->debug("Initializing frame buffer.");
		if (camera == nullptr)
		{
			frame_buffer = std::make_unique<FrameBuffer>(window->get_width(), window->get_height(), true, false);
		}
		else
		{
			const auto& dim = camera->transform.dimension;
			frame_buffer->resize(static_cast<int>(dim.x), static_cast<int>(dim.y));
		}

		// Force linear filtering
		glBindTexture(frame_buffer->texture->target, frame_buffer->texture->id);
		glTexParameteri(frame_buffer->texture->target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(frame_buffer->texture->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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

	void Renderer2::destroy_layers(void)
	{
		layers.clear();
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
		auto layer = get_layer(id);
		if (layer == nullptr)
		{
			log->warn("Invalid layer: {}", id);
		}
		else
		{
			layer->add(sprite);
		}
	}

	void Renderer2::remove_from_layer(const std::string& id, Sprite* sprite)
	{
		auto layer = get_layer(id);
		if (layer == nullptr)
		{
			log->warn("Invalid layer: {}", id);
		}
		else
		{
			layer->remove(sprite);
		}
	}

	void Renderer2::set_camera(std::unique_ptr<Camera2> camera)
	{
		this->camera = std::move(camera);
		if (frame_buffer == nullptr || frame_buffer->texture->w != this->camera->transform.dimension.x
			|| frame_buffer->texture->h != this->camera->transform.dimension.y)
		{
			initialize_frame_buffer();
		}
	}

	void Renderer2::render(void)
	{
		frame_buffer->bind();
		window->clear();

#if OPENGL_VERSION >= 30
		// This will pick up the change we made to camera transform. Since we're 
		// using uniform buffers, this will only be done once each frame.
		update_uniforms();
#endif

		// Scene pass
		for (auto& layer : layers)
		{
			if (layer->visible() && layer->stage == RenderStage::SCENE)
			{
				layer->render(this);
			}
		}

		frame_buffer->unbind();

		// Composite pass
		window->clear();
		switch_shader(composite_program);
		composite_program->set("u_aspect", camera->get_aspect_ratio());
		frame_buffer->texture->bind(0, composite_program);
		if (composite_binder)
			composite_binder(composite_program);

		quad->render(composite_program);

		// Overlay pass
		for (auto& layer : layers)
		{
			if (layer->visible() && layer->stage == RenderStage::OVERLAY)
			{
				layer->render(this);
			}
		}

		window->present();

#if OPENGL_VERSION < 30
		// invalidate active program to force uniforms rebind during
		// next frame
		active_program = 0;
#endif
	}

	void Renderer2::update_uniforms(void)
	{
#if OPENGL_VERSION >= 30
		// Update uniform buffers
		glBindBufferBase(GL_UNIFORM_BUFFER, UniformBuffer::CAMERA, uniform_buffers->buffers[UniformBuffer::CAMERA]);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraTransform2), &camera->transform, GL_STREAM_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, UniformBuffer::LIGHT, uniform_buffers->buffers[UniformBuffer::LIGHT]);
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
