#include "stdafx.h"
#include "shadercache.h"
#include "shaderprogram.h"
#include "log.h"

namespace dukat
{
	std::string ShaderCache::load_shader(const std::string& filename)
	{
		if (sources.count(filename) == 0)
		{
			// Load shader from file
			auto fqn = resource_dir + "/" + filename;
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
		std::string shaderLog(log);
		if (shaderLog.size() > 0)
		{
			logger << shaderLog;
		}
		else
		{
			logger << "OK" << std::endl;
		}

		if (!status)
		{
			throw std::runtime_error("Could not compile shader");
		}

		return shader;
	}

	GLuint ShaderCache::build_program(const std::string& vertexFile, const std::string& fragmentFile,
			const std::string& geometryFile)
	{
		GLuint set_program = glCreateProgram();
		GLuint vertexShader = build_shader(GL_VERTEX_SHADER, vertexFile);
		glAttachShader(set_program, vertexShader);

		// Geometry shader is optional
		GLuint geometryShader = -1;
		if (geometryFile != "")
		{
			geometryShader = build_shader(GL_GEOMETRY_SHADER, geometryFile);
			glAttachShader(set_program, geometryShader);
		}

		GLuint fragmentShader = build_shader(GL_FRAGMENT_SHADER, fragmentFile);
		glAttachShader(set_program, fragmentShader);
	
		// tell opengl the name of the output variable of the fragment shader
		// 0 means buffer 0, see http://www.opengl.org/wiki/GLAPI/glDrawBuffers
		glBindFragDataLocation(set_program, 0, "outColor");
		glLinkProgram(set_program);

		if (vertexShader >= 0)
		{
			glDetachShader(set_program, vertexShader);
			glDeleteShader(vertexShader);
		}
		if (fragmentShader >= 0)
		{
			glDetachShader(set_program, fragmentShader);
			glDeleteShader(fragmentShader);
		}
		if (geometryShader >= 0)
		{
			glDetachShader(set_program, geometryShader);
			glDeleteShader(geometryShader);
		}

		return set_program;
	}

	ShaderProgram* ShaderCache::get_program(const std::string& vertexFile, const std::string& fragmentFile,
		const std::string& geometryFile)
	{
		std::string key = vertexFile + "|" + fragmentFile + "|" + geometryFile;
		if (programs.count(key) == 0)
		{
			GLuint set_program = build_program(vertexFile, fragmentFile, geometryFile);
			programs[key] = std::make_unique<ShaderProgram>(set_program);
		}
		return programs[key].get();
	}
}