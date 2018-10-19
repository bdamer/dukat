#pragma once

#include <string>
#include <map>
#include <memory>

#ifndef OPENGL_VERSION
#include "version.h"
#endif // !OPENGL_VERSION

#include "shaderprogram.h"

namespace dukat
{
	class ShaderCache
	{
	private:
		static constexpr const char* pre_include = "#include";

#if OPENGL_CORE >= 30
		static constexpr auto glsl = "150";
#elif OPENGL_CORE > 0
		static constexpr auto glsl = "120";
#elif OPENGL_ES >= 30
		static constexpr auto glsl = "300es";
#else
		static constexpr auto glsl = "100es";
#endif

		std::map<std::string,std::unique_ptr<ShaderProgram>> programs;
		std::map<std::string, std::string> sources;
		const std::string resource_dir;
		// Loads the source of a shader from file.
		std::string load_shader(const std::string& filename);
		// Builds a new shader from a source file.
		GLuint build_shader(GLenum shaderType, const std::string& filename);
		// Builds a new program from a set of source files.
		GLuint build_program(const std::string& vertex_file, const std::string& fragement_file,
			const std::string& geometry_file = "");

	public:
		ShaderCache(const std::string& resource_dir) : resource_dir(resource_dir) { }
		~ShaderCache(void) { }

		// Returns a program for a set of shaders. If necessary, will create the program.
		ShaderProgram* get_program(const std::string& vertex_file, const std::string& fragement_file,
			const std::string& geometry_file = "");
	};
}

