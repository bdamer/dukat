#include "stdafx.h"
#include "log.h"
#include "perfcounter.h"
#include "renderer.h"
#include "shadercache.h"
#include "buffers.h"

namespace dukat
{
	Renderer::Renderer(Window* window, ShaderCache* shader_cache)
		: window(window), shader_cache(shader_cache), active_program(0), use_wireframe(false)
	{
		window->bind(this);
		enumerate_capabilities();
		test_capabilities();
		uniform_buffers = std::make_unique<GenericBuffer>(2);

		// Default settings
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

		// Enable back-face culling
		glFrontFace(GL_CCW);
#ifdef _DEBUG
		glPolygonMode(GL_BACK, GL_LINE);
		glPolygonMode(GL_FRONT, GL_FILL);
#else
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
#endif
	}

	Renderer::~Renderer(void)
	{
		window->unbind(this);
	}

	void Renderer::enumerate_capabilities(void)
	{
		auto list = (const char*)(glGetString(GL_EXTENSIONS));
		for (auto ptr = list; *ptr != '\0'; ptr++)
		{
			if (*ptr != ' ')
				continue;

			std::string extension(list, ptr - list);
			extensions.insert(extension);
			list = ptr + 1;
		}
		logger << "Found " << extensions.size() << " extensions." << std::endl;
	}

	void Renderer::test_capabilities(void)
	{
		GLint int_val;
		logger << "Testing rendering capabilities" << std::endl;

		auto shader_version = std::string((char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
		logger << "GL_SHADING_LANGUAGE_VERSION: " << shader_version << std::endl;
		logger << "Geometry shader support: " << (GLEW_ARB_geometry_shader4 ? "yes" : "no") << std::endl;

		glGetIntegerv(GL_MAX_TEXTURE_UNITS, &int_val);
		logger << "GL_MAX_TEXTURE_UNITS: " << int_val << std::endl;
		assert(int_val >= Renderer::max_texture_units);

		glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &int_val);
		logger << "GL_MAX_UNIFORM_BUFFER_BINDINGS: " << int_val << std::endl;
		assert(int_val >= UniformBuffer::_COUNT);

		glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &int_val);
		logger << "GL_MAX_UNIFORM_BLOCK_SIZE: " << int_val << std::endl;

		auto supported = is_ext_supported("GL_EXT_texture_filter_anisotropic");
		logger << "GL_EXT_texture_filter_anisotropic: " << supported << std::endl;
		assert(supported);

		

		// test capabilities as needed...
	}

	void Renderer::resize(int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void Renderer::switch_shader(ShaderProgram* program)
	{
		assert(program != nullptr);
		if (active_program != program)
		{
			glUseProgram(program->id);
			perfc.inc(PerformanceCounter::SHADERS);
			// re-bind uniform blocks
			glUniformBlockBinding(program->id, glGetUniformBlockIndex(program->id, Renderer::uf_camera), UniformBuffer::CAMERA);
			glBindBufferBase(GL_UNIFORM_BUFFER, UniformBuffer::CAMERA, uniform_buffers->buffers[0]);
			glUniformBlockBinding(program->id, glGetUniformBlockIndex(program->id, Renderer::uf_light), UniformBuffer::LIGHT);
			glBindBufferBase(GL_UNIFORM_BUFFER, UniformBuffer::LIGHT, uniform_buffers->buffers[1]);
			this->active_program = program;
		}
	}

	void Renderer::toggle_wireframe(void)
	{
		use_wireframe = !use_wireframe;
		if (use_wireframe)
		{
			glPolygonMode(GL_FRONT, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT, GL_FILL);
		}
	}
}