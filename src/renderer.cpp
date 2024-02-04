#include "stdafx.h"
#include <dukat/color.h>
#include <dukat/log.h>
#include <dukat/perfcounter.h>
#include <dukat/renderer.h>
#include <dukat/shadercache.h>
#include <dukat/surface.h>
#include <dukat/sysutil.h>
#include <dukat/buffers.h>

namespace dukat
{
	Renderer::Renderer(Window* window, ShaderCache* shader_cache)
		: window(window), shader_cache(shader_cache), active_program(0), show_wireframe(false), blending(false)
	{
		test_capabilities();
		uniform_buffers = std::make_unique<GenericBuffer>(static_cast<int>(UniformBuffer::_count));
		// Default settings
		set_clear_color(Color{ 0.0f, 0.0f, 0.0f, 0.0f });
		// Enable back-face culling
		glFrontFace(GL_CCW);
		set_backface_culling(true);
		gl_check_error();
		window->subscribe(this, events::WindowResized);
		// Present once to show blank screen during initial load
		window->present();
	}

	Renderer::~Renderer(void)
	{
		window->unsubscribe(this, events::WindowResized);
	}

	void Renderer::test_capabilities(void)
	{
		GLint int_val;
	
		const auto vendor = glGetString(GL_VENDOR);
		const auto model = glGetString(GL_RENDERER);
		log->debug("Testing rendering capabilities: {} - {}", vendor, model);

		const auto shader_version = glGetString(GL_SHADING_LANGUAGE_VERSION);
		log->trace("GL_SHADING_LANGUAGE_VERSION: {}", shader_version);
		
		// Check capabilities
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &int_val);
		log->trace("GL_MAX_TEXTURE_IMAGE_UNITS: {}", int_val);
		assert(int_val >= Renderer::max_texture_units);
		glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &int_val);
		log->trace("GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS: {}", int_val);
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &int_val);
		log->trace("GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS: {}", int_val);

#if OPENGL_VERSION >= 30
		glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &int_val);
		log->trace("GL_MAX_UNIFORM_BUFFER_BINDINGS: {}", int_val);
		assert(int_val >= static_cast<int>(UniformBuffer::_count));
		glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &int_val);
		log->trace("GL_MAX_UNIFORM_BLOCK_SIZE: {}", int_val);
        glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &int_val);
        log->trace("GL_MAX_VERTEX_UNIFORM_BLOCKS: {}", int_val);
#endif

#if OPENGL_CORE >= 42
		// TODO: validate with ctx 4.x
		glGetInternalformativ(GL_TEXTURE_2D, GL_RGBA8, GL_INTERNALFORMAT_PREFERRED, 1, &int_val);
		log->trace("Preferred format for GL_RGBA8: {}", int_val);
		glGetInternalformativ(GL_TEXTURE_2D, GL_RGB8, GL_INTERNALFORMAT_PREFERRED, 1, &int_val);
		log->trace("Preferred format for GL_RGB8: {}", int_val);
		glGetInternalformativ(GL_TEXTURE_2D, GL_RGBA16, GL_INTERNALFORMAT_PREFERRED, 1, &int_val);
		log->trace("Preferred format for GL_RGBA16: {}", int_val);
		glGetInternalformativ(GL_TEXTURE_2D, GL_RGB16, GL_INTERNALFORMAT_PREFERRED, 1, &int_val);
		log->trace("Preferred format for GL_RGB16: {}", int_val);
#endif

#if OPENGL_CORE >= 43
		glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, &int_val);
		log->trace("GL_MAX_UNIFORM_LOCATIONS: {}", int_val);
#endif

		// Check supported extensions
		log->trace("GL_EXT_geometry_shader4: {}",
			(SDL_GL_ExtensionSupported("GL_EXT_geometry_shader4") ? "yes" : "no"));
		log->trace("GL_EXT_texture_filter_anisotropic: {}",
			(SDL_GL_ExtensionSupported("GL_EXT_texture_filter_anisotropic") ? "yes" : "no"));
		log->trace("GL_EXT_vertex_array_object: {}",
			(SDL_GL_ExtensionSupported("GL_EXT_vertex_array_object") ? "yes" : "no"));
		log->trace("GL_EXT_framebuffer_object: {}",
			(SDL_GL_ExtensionSupported("GL_EXT_framebuffer_object") ? "yes" : "no"));

#ifdef _DEBUG
		gl_check_error();
#endif
	}

	void Renderer::resize_window(int w, int h)
	{
		reset_viewport();
	}

	void Renderer::switch_shader(ShaderProgram* program)
	{
		assert(program != nullptr);
		if (active_program != program)
		{
			glUseProgram(program->id);
			perfc.inc(PerformanceCounter::SHADERS);
			this->active_program = program;
#if OPENGL_VERSION >= 30
			// re-bind uniform blocks
			if (program->bind(Renderer::uf_camera, static_cast<GLuint>(UniformBuffer::Camera)))
				glBindBufferBase(GL_UNIFORM_BUFFER, static_cast<GLuint>(UniformBuffer::Camera), uniform_buffers->buffers[static_cast<int>(UniformBuffer::Camera)]);
			if (program->bind(Renderer::uf_light, static_cast<GLuint>(UniformBuffer::Light)))
				glBindBufferBase(GL_UNIFORM_BUFFER, static_cast<GLuint>(UniformBuffer::Light), uniform_buffers->buffers[static_cast<int>(UniformBuffer::Light)]);
#else
			// update all uniforms
			update_uniforms();
#endif
		}
	}

	void Renderer::bind_uniform(UniformBuffer buffer, GLsizeiptr size, const GLvoid* data)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, static_cast<GLuint>(buffer), uniform_buffers->buffers[static_cast<int>(buffer)]);
		glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STREAM_DRAW);
	}

	std::unique_ptr<Surface> Renderer::copy_screen_buffer(void)
	{
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		auto surface = std::make_unique<Surface>(viewport[2], viewport[3], SDL_PIXELFORMAT_RGB24);
		glReadPixels(viewport[0], viewport[1], viewport[2], viewport[3], GL_RGB, GL_UNSIGNED_BYTE, surface->get_surface()->pixels);
		surface->flip_vertical();
		return std::move(surface);
	}

	void Renderer::receive(const Message& msg)
	{
		switch (msg.event)
		{
		case events::WindowResized:
			resize_window(*msg.get_param1<int>(), *msg.get_param2<int>());
			break;
		}
	}

	void Renderer::set_wireframe(bool wireframe)
	{
		show_wireframe = wireframe;
	}

	void Renderer::set_backface_culling(bool backface_culling)
	{
		this->backface_culling = backface_culling;
		if (backface_culling)
		{
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
		}
		else
		{
			glDisable(GL_CULL_FACE);
#ifdef OPENGL_CORE
			glPolygonMode(GL_BACK, GL_LINE);
			glPolygonMode(GL_FRONT, GL_FILL);
#endif
		}
	}

	void Renderer::set_blending(bool blending)
	{
		this->blending = blending;
		if (blending)
		{
			glEnable(GL_BLEND);
		}
		else
		{
			glDisable(GL_BLEND);
		}
	}

	void Renderer::set_clear_color(const Color & clr)
	{
		glClearColor(clr.r, clr.g, clr.b, clr.a);
	}

	void Renderer::reset_viewport(void)
	{
		glViewport(0, 0, window->get_width(), window->get_height());
	}
}