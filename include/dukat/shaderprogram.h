#pragma once

#include <unordered_map>
#include <string>

#include "matrix4.h"
#include "renderer.h"
#include "vector3.h"

namespace dukat
{
	class ShaderProgram
	{
	private:
		std::unordered_map<std::string,GLint> attributes;
		std::unordered_map<std::string,GLuint> uniforms;
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
		inline void set(const std::string& name, GLfloat* val) { glUniform4fv(attributes[name], 1, val); }
		inline void set(GLint index, GLfloat val0) { glUniform1f(index, val0); }
		inline void set(GLint index, GLfloat val0, GLfloat val1) { glUniform2f(index, val0, val1); }
		inline void set(GLint index, GLfloat val0, GLfloat val1, GLfloat val2) { glUniform3f(index, val0, val1, val2); }
		inline void set(GLint index, GLfloat val0, GLfloat val1, GLfloat val2, GLfloat val3) { glUniform4f(index, val0, val1, val2, val3); }
		inline void set(GLint index, GLfloat* val) { glUniform4fv(index, 1, val); }
		inline void set_vector3(const std::string& name, const Vector3& v) { glUniform4f(attributes[name], v.x, v.y, v.z, v.w); }
		inline void set_matrix4(const std::string& name, const Matrix4& matrix) { glUniformMatrix4fv(attributes[name], 1, false, matrix.m); }
		inline void set_matrix4(const std::string& name, GLfloat* matrix) { glUniformMatrix4fv(attributes[name], 1, false, matrix); }
		void bind(const std::string& block_name, Renderer::UniformBuffer ub);
	};
}