#pragma once

#include <GL/glew.h>
#include <unordered_map>
#include <string>

namespace dukat
{
	class ShaderProgram
	{
	private:
		std::unordered_map<std::string,GLint> attributes;
		void index_attributes(void);

	public:
		const GLuint id;

		ShaderProgram(GLuint id);
		~ShaderProgram(void);

		GLint attr(const std::string& name);
	};
}