#include "stdafx.h"
#include <dukat/shadercache.h>
#include <dukat/shaderprogram.h>
#include <dukat/log.h>
#include <dukat/sysutil.h>
#include <dukat/assetloader.h>

namespace dukat
{
	std::string ShaderCache::load_shader(const std::string& filename)
	{
		if (sources.count(filename) == 0)
		{
			// Load shader from file
			auto fqn = resource_dir + "/" + glsl + "/" + filename;
			AssetLoader loader;
			std::stringstream in;
			loader.load_text(fqn, in);

			std::stringstream ss;
			std::string line;
			int lineNum = 0;
			while (std::getline(in, line))
			{
				lineNum++;
				if (line.find(pre_include) == 0)
				{
					auto idx1 = line.find('"') + 1;
					auto idx2 = line.find('"', idx1);
					if (idx1 < 1 || idx2 < 1)
					{
						std::stringstream ess;
						ess << "Error processing file " << filename << " [" << lineNum << "]: " << line;
						throw std::runtime_error(ess.str());
					}
					auto include = line.substr(idx1, idx2 - idx1);
					ss << load_shader(include);
				}
				else
				{
					ss << line << std::endl;
				}
			}
			
			sources[filename] = ss.str();
		}
		return sources[filename];
	}

	GLuint ShaderCache::build_shader(GLenum shader_type, const std::string& filename)
	{
		log->debug("Compiling shader [{}]", filename);
		GLuint shader = glCreateShader(shader_type);
		auto src = load_shader(filename);
		auto srcPtr = src.c_str();
		glShaderSource(shader, 1, &srcPtr, NULL);

		// Compile and check result
		glCompileShader(shader);
		GLint status;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

		// Print compilation output, if any
		char msg[1024];
		glGetShaderInfoLog(shader, 1024, NULL, msg);
		std::string shader_log(msg);
		if (shader_log.size() > 0)
		{
			log->warn("Failed to compile shader: {}", shader_log);
		}

		if (!status)
		{
			throw std::runtime_error("Could not compile shader.");
		}

		return shader;
	}

	GLuint ShaderCache::build_program(const std::string& vertex_file, const std::string& fragment_file,
			const std::string& geometry_file)
	{
		auto program = glCreateProgram();
		auto vertex_shader = build_shader(GL_VERTEX_SHADER, vertex_file);
		glAttachShader(program, vertex_shader);

#if (OPENGL_CORE >= 30) || (OPENGL_ES >= 32)
		// Geometry shader is optional
		auto geometry_shader = 0;
		if (geometry_file != "")
		{
			geometry_shader = build_shader(GL_GEOMETRY_SHADER, geometry_file);
			glAttachShader(program, geometry_shader);
		}
#endif

		auto fragment_shader = build_shader(GL_FRAGMENT_SHADER, fragment_file);
		glAttachShader(program, fragment_shader);

#if (OPENGL_CORE >= 30)
		// tell opengl the name of the output variable of the fragment shader
		// 0 means buffer 0, see http://www.opengl.org/wiki/GLAPI/glDrawBuffers
		glBindFragDataLocation(program, 0, "outColor");
#endif
		glLinkProgram(program);

		if (vertex_shader > 0)
		{
			glDetachShader(program, vertex_shader);
			glDeleteShader(vertex_shader);
		}
		if (fragment_shader > 0)
		{
			glDetachShader(program, fragment_shader);
			glDeleteShader(fragment_shader);
		}
#if (OPENGL_CORE >= 30) || (OPENGL_ES >= 32)
		if (geometry_shader > 0)
		{
			glDetachShader(program, geometry_shader);
			glDeleteShader(geometry_shader);
		}
#endif

#ifdef _DEBUG
		gl_check_error();
#endif
		return program;
	}

	ShaderProgram* ShaderCache::get_program(const std::string& vertex_file, const std::string& fragment_file,
		const std::string& geometry_file)
	{
		std::string key = vertex_file + "|" + fragment_file + "|" + geometry_file;
		if (programs.count(key) == 0)
		{
			auto program = build_program(vertex_file, fragment_file, geometry_file);
			programs[key] = std::make_unique<ShaderProgram>(program);
		}
		return programs[key].get();
	}
}