#include "stdafx.h"
#include "shaderprogram.h"
#include "perfcounter.h"
#include "sysutil.h"

namespace dukat
{
	ShaderProgram::ShaderProgram(GLuint id) : id(id)
	{
		index_attributes();
	}

	ShaderProgram::~ShaderProgram(void)
	{
		if (id != 0)
		{
			glDeleteProgram(id);
		}
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
			{
				attributes[buffer] = location;
			}
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
			{
				attributes[buffer] = location;
			}
		}

#ifdef _DEBUG
		gl_check_error();
#endif
	}

	GLint ShaderProgram::attr(const std::string & name)
	{
		if (attributes.count(name) == 0)
		{
			return -1;
		}
		else
		{
			return attributes[name];
		}
	}
}