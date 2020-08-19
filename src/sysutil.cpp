#include "stdafx.h"
#include <dukat/sysutil.h>
#include <dukat/log.h>
#include <dukat/surface.h>

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
}