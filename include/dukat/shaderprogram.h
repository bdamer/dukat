#pragma once

#include <string>
#include <robin_hood.h>

#include "matrix4.h"
#include "renderer.h"
#include "vector3.h"

namespace dukat
{
	class ShaderProgram
	{
	private:
		robin_hood::unordered_map<std::string,GLint> attributes;
		robin_hood::unordered_map<std::string,GLuint> uniforms;
		void index_attributes(void);

	public:
		const GLuint id;
		const std::string vertex_file;
		const std::string fragment_file;
		const std::string geometry_file;

		ShaderProgram(GLuint id, const std::string& vertex_file, const std::string& fragment_file, const std::string& geometry_file);
		~ShaderProgram(void);

		GLint attr(const std::string& name);
		inline void set(const std::string& name, GLfloat val0) { assert(attributes.count(name) > 0); glUniform1f(attributes.at(name), val0); }
		inline void set(const std::string& name, GLfloat val0, GLfloat val1) { assert(attributes.count(name) > 0); glUniform2f(attributes.at(name), val0, val1); }
		inline void set(const std::string& name, GLfloat val0, GLfloat val1, GLfloat val2) { assert(attributes.count(name) > 0); glUniform3f(attributes.at(name), val0, val1, val2); }
		inline void set(const std::string& name, GLfloat val0, GLfloat val1, GLfloat val2, GLfloat val3) { assert(attributes.count(name) > 0); glUniform4f(attributes.at(name), val0, val1, val2, val3); }
		inline void set(const std::string& name, GLfloat* val) { assert(attributes.count(name) > 0); glUniform4fv(attributes.at(name), 1, val); }
		inline void set(GLint index, GLfloat val0) { glUniform1f(index, val0); }
		inline void set(GLint index, GLfloat val0, GLfloat val1) { glUniform2f(index, val0, val1); }
		inline void set(GLint index, GLfloat val0, GLfloat val1, GLfloat val2) { glUniform3f(index, val0, val1, val2); }
		inline void set(GLint index, GLfloat val0, GLfloat val1, GLfloat val2, GLfloat val3) { glUniform4f(index, val0, val1, val2, val3); }
		inline void set(GLint index, GLfloat* val) { glUniform4fv(index, 1, val); }
		inline void set_vector3(const std::string& name, const Vector3& v) { assert(attributes.count(name) > 0); glUniform4f(attributes.at(name), v.x, v.y, v.z, v.w); }
		inline void set_matrix4(const std::string& name, const Matrix4& matrix) { assert(attributes.count(name) > 0); glUniformMatrix4fv(attributes.at(name), 1, false, matrix.m); }
		inline void set_matrix4(const std::string& name, GLfloat* matrix) { assert(attributes.count(name) > 0); glUniformMatrix4fv(attributes.at(name), 1, false, matrix); }
		// Binds a named uniform buffer. Returns true if the operation was successful.
		bool bind(const std::string& block_name, GLuint block_binding);
	};
}