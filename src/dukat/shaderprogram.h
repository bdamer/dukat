#pragma once

#include <GL/glew.h>
#include <map>
#include <string>

namespace dukat
{
	// TODO: revisit use of std::map - hashmap might be better
	class ShaderProgram
	{
	private:
		std::map<std::string,GLint> attributes;
		void index_attributes(void);

	public:
		const GLuint id;

		ShaderProgram(GLuint id);
		~ShaderProgram(void);

		GLint attr(const std::string& name);
	};
}