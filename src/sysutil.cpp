#include "stdafx.h"
#include <dukat/sysutil.h>
#include <dukat/log.h>

namespace dukat
{
	void gl_check_error(void)
	{
		GLenum err;
		while (GL_NO_ERROR != (err = glGetError())) 
		{
			std::string error;
			switch (err)
			{
				case GL_INVALID_ENUM:
					error = "GL_INVALID_ENUM";
					break;
				case GL_INVALID_VALUE:
					error = "GL_INVALID_VALUE";
					break;
				case GL_INVALID_OPERATION:
					error = "GL_INVALID_OPERATION";
					break;
				case GL_INVALID_FRAMEBUFFER_OPERATION:
					error = "GL_INVALID_FRAMEBUFFER_OPERATION";
					break;
				case GL_OUT_OF_MEMORY:
					error = "GL_OUT_OF_MEMORY";
					break;
#ifndef __ANDROID__
				case GL_STACK_UNDERFLOW:
					error = "GL_STACK_UNDERFLOW";
					break;
				case GL_STACK_OVERFLOW:
					error = "GL_STACK_OVERFLOW";
					break;
#endif
				default:
					error = "unknown";
					break;
			}
			log->error("glError: {}", error);
		}
	}

	void crash_dump(const std::vector<std::string>& paths)
	{
		const auto now = time(nullptr);
		const auto dir_name = "crash_" + std::to_string(now);
		if (!create_directory(dir_name))
		{
			log->error("Failed to create crash dir: {}", dir_name);
			return;
		}

		for (const auto& it : paths)
		{
			if (file_exists(it))
			{
				const auto name = file_name(it);
				copy_file(it, dir_name + "/" + name);
			}
		}
	}

	bool file_exists(const std::string& path)
	{
		struct stat tmp;
		return stat(path.c_str(), &tmp) == 0;
	}

	bool directory_exists(const std::string& path)
	{
		struct stat tmp;
		return (stat(path.c_str(), &tmp) == 0) && (tmp.st_mode & S_IFDIR);
	}

	bool create_directory(const std::string& path)
	{
#ifdef WIN32
		auto res = _mkdir(path.c_str());
#else
		mode_t mode = 0755;
		auto res = mkdir(path.c_str(), mode);
#endif
		return (res == 0);
	}

	bool copy_file(const std::string& source, const std::string& dest)
	{
		std::ifstream src(source, std::ios::binary);
		if (!src)
			return false;

		std::ofstream dst(dest, std::ios::binary);
		if (!dst)
			return false;

		dst << src.rdbuf();

		return true;
	}
}