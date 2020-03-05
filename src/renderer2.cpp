#include "stdafx.h"
#include <dukat/camera2.h>
#include <dukat/log.h>
#include <dukat/renderer2.h>
#include <dukat/renderlayer2.h>
#include <dukat/shadercache.h>
#include <dukat/sprite.h>
#include <dukat/vertextypes2.h>
#include <dukat/sysutil.h>
#include <dukat/meshbuilder2.h>
#include <dukat/meshdata.h>
#include <dukat/textureutil.h>

namespace dukat
{
	Renderer2::Renderer2(Window* window, ShaderCache* shader_cache) : Renderer(window, shader_cache), 
		render_effects(true), render_sprites(true), render_particles(true), render_text(true)
	{
		// Enable transparency
		set_blending(true);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// Disable depth test - we'll have to take care of rendering order ourselves
		glDisable(GL_DEPTH_TEST);
#ifdef OPENGL_CORE
        // Enable gl_PointSize instruction in shader
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
#endif

		initialize_sprite_buffers();
		initialize_particle_buffers();
		initialize_frame_buffer();

		MeshBuilder2 builder;
		quad = builder.build_textured_quad();

		light.position = Vector3(0.0f, 0.0f, 0.0f);
		gl_check_error();
	}

	void Renderer2::initialize_sprite_buffers(void)
	{
		// Vertex order:
		//	0 2
		//	1 3
		Vertex2PT vertices[] = {
			{ -0.5f, -0.5f, 0.0f, 0.0f },	// top-left
			{ -0.5f,  0.5f, 0.0f, 1.0f },	// bottom-left
			{  0.5f, -0.5f, 1.0f, 0.0f },	// top-right
			{  0.5f,  0.5f, 1.0f, 1.0f }	// bottom-right
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
		log->trace("Initializing frame buffer.");
		if (camera == nullptr)
		{
			frame_buffer = std::make_unique<FrameBuffer>(window->get_width(), window->get_height(), true, false, TextureFilterProfile::ProfileNearest);
		}
		else
		{
			const auto& dim = camera->transform.dimension;
			frame_buffer->resize(static_cast<int>(dim.x), static_cast<int>(dim.y));
		}
	}

	RenderLayer2* Renderer2::create_layer(const std::string& id, float priority, float parallax, bool has_render_target)
	{
		log->debug("Creating layer: {} [{} {}]", id, priority, parallax);
		auto layer = std::make_unique<RenderLayer2>(shader_cache, 
			sprite_buffer.get(), particle_buffer.get(), id, priority, parallax);
		if (has_render_target)
		{
			// create render target compatible with current frame buffer
			auto texture = std::make_unique<Texture>(frame_buffer->width, frame_buffer->height);
			frame_buffer->initialize_draw_buffer(texture.get());
			layer->set_render_target(std::move(texture));
		}
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

	RenderLayer2* Renderer2::create_overlay_layer(const std::string& id, float priority)
	{
		auto layer = create_layer(id, priority);
		layer->stage = RenderStage::OVERLAY;
		layer->set_composite_program(nullptr); // by default overlay doesn't use compositing
		return layer;
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

	void Renderer2::render_layer(RenderLayer2& layer)
	{
		// Each layer can either render directly to the global screen buffer,
		// or alternatively render to a dedicated frame buffer followed by
		// a composite pass to merge the frame buffer into the screen buffer
		auto comp_program = layer.get_composite_program();
		auto render_target = layer.get_render_target();
		if (comp_program != nullptr)
		{
			frame_buffer->bind();
			if (render_target != nullptr)
				frame_buffer->attach_draw_buffer(render_target);
			clear();
		}

		layer.render(this);

		// Composite pass
		if (comp_program != nullptr)
		{
			if (render_target != nullptr)
				frame_buffer->detach_draw_buffer();
			frame_buffer->unbind();
			reset_viewport();
			switch_shader(comp_program);
			auto id = comp_program->attr("u_aspect");
			if (id != -1)
				comp_program->set(id, camera->get_aspect_ratio());
			if (render_target != nullptr)
				render_target->bind(0, comp_program);
			else
				frame_buffer->texture->bind(0, comp_program);
			const auto& binder = layer.get_composite_binder();
			if (binder)
				binder(comp_program);
			// Finally, render composite image to screen buffer
			quad->render(comp_program);
		}
	}

	void Renderer2::render(void)
	{
		// Call glFinish to avoid buffer updates on Intel GPU.
		// The goal is to wait until all pending render operations of
		// the previous frame have finished.
		glFinish();

		clear();

#if OPENGL_VERSION >= 30
		// This will pick up the change we made to camera transform. Since we're 
		// using uniform buffers, this will only be done once each frame.
		update_uniforms();
#endif
		// Scene pass
		for (auto& layer : layers)
		{
			if (!layer->visible() || layer->stage != RenderStage::SCENE)
				continue;
			render_layer(*layer);
		}

		// Overlay pass
		for (auto& layer : layers)
		{
			if (!layer->visible() || layer->stage != RenderStage::OVERLAY)
				continue;
			render_layer(*layer);
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
