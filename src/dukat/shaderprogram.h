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
		inline void set(const std::string& name, GLfloat val0) { glUniform1f(attributes[name], val0); }
		inline void set(const std::string& name, GLfloat val0, GLfloat val1) { glUniform2f(attributes[name], val0, val1); }
		inline void set(const std::string& name, GLfloat val0, GLfloat val1, GLfloat val2) { glUniform3f(attributes[name], val0, val1, val2); }
		inline void set(const std::string& name, GLfloat val0, GLfloat val1, GLfloat val2, GLfloat val3) { glUniform4f(attributes[name], val0, val1, val2, val3); }
	};
}