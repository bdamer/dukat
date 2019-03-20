#include "stdafx.h"
#include <dukat/sysutil.h>
#include <dukat/log.h>
#include <dukat/surface.h>

namespace dukat
{
	void sdl_check_result(int res, const std::string& operation)
	{
		if (res < 0)
		{
			std::ostringstream ss;
			ss << "Failed to " << operation << ": " << SDL_GetError() << std::endl;
			throw std::runtime_error(ss.str());
		}
	}

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

	std::string read_pstring(std::istream& is)
	{
		size_t size;
		is.read(reinterpret_cast<char*>(&size), sizeof(uint32_t));
		std::vector<char> buffer;
		buffer.resize(size);
		is.read(buffer.data(), size);
		return std::string(buffer.data());
	}

	size_t write_pstring(std::ostream& os, const std::string& str)
	{
		size_t size = str.length() + 1;
		os.write(reinterpret_cast<const char*>(&size), sizeof(uint32_t));
		os.write(str.c_str(), size);
		return size + sizeof(uint32_t);
	}

	void save_screenshot(const std::string& filename)
	{
		log->info("Saving screenshot to: {}", filename);
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		Surface surface(viewport[2], viewport[3], SDL_PIXELFORMAT_RGB24);
		glReadPixels(viewport[0], viewport[1], viewport[2], viewport[3], GL_RGB, GL_UNSIGNED_BYTE, surface.get_surface()->pixels);
		surface.flip_vertical();
		surface.save_to_file(filename);
	}

	uint32_t compute_hash(const std::string& s)
	{
		// pad size to multiple of 4
		int size = (s.length() + 3) & ~0x03;
		std::vector<unsigned char> buffer(size);
		std::transform(s.begin(), s.end(), buffer.begin(), toupper);

		// process characters in groups of 4 bytes
		uint32_t res = 0;
		for (int i = 0; i < size; i += 4)
		{
			res = (res << 1) | (res >> 31); // rotate left by 1
			res += (buffer[i+3] << 24) + (buffer[i+2] << 16) + (buffer[i+1] << 8) + buffer[i]; // accumulate
		}
		return res;
	}
}