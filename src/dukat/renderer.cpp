#include "stdafx.h"
#include "log.h"
#include "perfcounter.h"
#include "renderer.h"
#include "shadercache.h"
#include "buffers.h"

namespace dukat
{
	Renderer::Renderer(Window* window, ShaderCache* shader_cache)
		: window(window), shader_cache(shader_cache), active_program(0)
	{
		window->bind(this);
		test_capabilities();
		uniform_buffers = std::make_unique<GenericBuffer>(2);
	}

	Renderer::~Renderer(void)
	{
		window->unbind(this);
	}

	void Renderer::test_capabilities(void)
	{
		logger << "Testing rendering capabilities" << std::endl;
		GLint param;
		glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &param);
		logger << "GL_MAX_UNIFORM_BUFFER_BINDINGS: " << param << std::endl;
		assert(param >= UniformBuffer::_COUNT);

		glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &param);
		logger << "GL_MAX_UNIFORM_BLOCK_SIZE: " << param << std::endl;

		auto shader_version = std::string((char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
		logger << "GL_SHADING_LANGUAGE_VERSION: " << shader_version << std::endl;

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
}