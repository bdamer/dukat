#include "stdafx.h"
#include <dukat/log.h>
#include <dukat/shaderprogram.h>
#include <dukat/perfcounter.h>
#include <dukat/sysutil.h>

namespace dukat
{
	ShaderProgram::ShaderProgram(GLuint id, const std::string& vertex_file, const std::string& fragment_file, const std::string& geometry_file)
		: id(id), vertex_file(vertex_file), fragment_file(fragment_file), geometry_file(geometry_file)
	{
		index_attributes();
	}

	ShaderProgram::~ShaderProgram(void)
	{
		if (id != 0)
			glDeleteProgram(id);
	}

	void ShaderProgram::index_attributes(void)
	{
	    const auto buffer_size = 256;
		GLchar buffer[buffer_size];
		// query and index uniforms
		GLint count;
		glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &count);
		for (auto i = 0; i < count; i++)
		{
			GLint size;
			GLenum type;
			GLsizei length;
			glGetActiveUniform(id, i, buffer_size, &length, &size, &type, buffer);
			auto location = glGetUniformLocation(id, buffer);
			if (location >= 0)
				attributes.emplace(buffer, location);
		}

		// query and index attributes
		glGetProgramiv(id, GL_ACTIVE_ATTRIBUTES, &count);
		for (auto i = 0; i < count; i++)
		{
			GLint size;
			GLenum type;
			GLsizei length;
			glGetActiveAttrib(id, i, buffer_size, &length, &size, &type, buffer);
			auto location = glGetAttribLocation(id, buffer);
			if (location >= 0)
				attributes.emplace(buffer, location);
		}

#ifdef _DEBUG
		gl_check_error();
#endif
	}

	GLint ShaderProgram::attr(const std::string& name)
	{
		if (attributes.count(name) == 0)
		{
			return -1;
		}
		else
		{
			return attributes.at(name);
		}
	}

	bool ShaderProgram::bind(const std::string& block_name, GLuint block_binding)
	{
		if (uniforms.count(block_name) == 0) // determine index on first call
			uniforms.emplace(block_name, glGetUniformBlockIndex(id, block_name.c_str()));
		const auto& idx = uniforms.at(block_name);
		if (idx != GL_INVALID_INDEX)
		{
			glUniformBlockBinding(id, idx, block_binding);
			return true;
		}
		else
		{
			return false;
		}
	}
}