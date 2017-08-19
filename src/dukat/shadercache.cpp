#include "stdafx.h"
#include "shadercache.h"
#include "shaderprogram.h"
#include "log.h"
#include "dukat.h"
#include "sysutil.h"

namespace dukat
{
	std::string ShaderCache::load_shader(const std::string& filename)
	{
		if (sources.count(filename) == 0)
		{
			// Load shader from file
#if OPENGL_VERSION >= 30
			auto fqn = resource_dir + "/150/" + filename;
#else
			auto fqn = resource_dir + "/120/" + filename;
#endif
			std::ifstream in(fqn);
			if (!in)
			{
				auto err = "Could not load file: " + fqn;
				throw std::runtime_error(err);
			}

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

	GLuint ShaderCache::build_shader(GLenum shaderType, const std::string& filename)
	{
		logger << "Compiling shader [" << filename << "]: ";
		GLuint shader = glCreateShader(shaderType);
		auto src = load_shader(filename);
		auto srcPtr = src.c_str();
		glShaderSource(shader, 1, &srcPtr, NULL);

		// Compile and check result
		glCompileShader(shader);
		GLint status;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

		// Print compilation output, if any
		char log[1024];
		glGetShaderInfoLog(shader, 1024, NULL, log);
		std::string shader_log(log);
		if (shader_log.size() > 0)
		{
			logger << shader_log << std::endl;
		}
		else
		{
			logger << "OK" << std::endl;
		}

		if (!status)
		{
			throw std::runtime_error("Could not compile shader.");
		}

		return shader;
	}

	GLuint ShaderCache::build_program(const std::string& vertexFile, const std::string& fragmentFile,
			const std::string& geometryFile)
	{
		GLuint program = glCreateProgram();
		GLuint vertexShader = build_shader(GL_VERTEX_SHADER, vertexFile);
		glAttachShader(program, vertexShader);

#if OPENGL_VERSION >= 30
		// Geometry shader is optional
		GLuint geometryShader = 0;
		if (geometryFile != "")
		{
			geometryShader = build_shader(GL_GEOMETRY_SHADER, geometryFile);
			glAttachShader(program, geometryShader);
		}
#endif

		GLuint fragmentShader = build_shader(GL_FRAGMENT_SHADER, fragmentFile);
		glAttachShader(program, fragmentShader);

#if OPENGL_VERSION >= 30
		// tell opengl the name of the output variable of the fragment shader
		// 0 means buffer 0, see http://www.opengl.org/wiki/GLAPI/glDrawBuffers
		glBindFragDataLocation(program, 0, "outColor");
#endif
		glLinkProgram(program);

		if (vertexShader > 0)
		{
			glDetachShader(program, vertexShader);
			glDeleteShader(vertexShader);
		}
		if (fragmentShader > 0)
		{
			glDetachShader(program, fragmentShader);
			glDeleteShader(fragmentShader);
		}
#if OPENGL_VERSION >= 30
		if (geometryShader > 0)
		{
			glDetachShader(program, geometryShader);
			glDeleteShader(geometryShader);
		}
#endif

		gl_check_error();
		return program;
	}

	ShaderProgram* ShaderCache::get_program(const std::string& vertexFile, const std::string& fragmentFile,
		const std::string& geometryFile)
	{
		std::string key = vertexFile + "|" + fragmentFile + "|" + geometryFile;
		if (programs.count(key) == 0)
		{
			GLuint program = build_program(vertexFile, fragmentFile, geometryFile);
			programs[key] = std::make_unique<ShaderProgram>(program);
		}
		return programs[key].get();
	}
}