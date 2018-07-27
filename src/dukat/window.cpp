#include "stdafx.h"
#include "log.h"
#include "sysutil.h"
#include "window.h"

namespace dukat
{
	Window::Window(int width, int height, bool fullscreen, bool vsync, bool msaa)
		: width(width), height(height), fullscreen(fullscreen)
	{
		// Need to request MSAA buffers before creating window
		if (msaa) 
		{
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 1);
		}

		// Create OpenGL context with desired profile version
#if OPENGL_VERSION >= 30
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#else
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

		// Create the window
		if (fullscreen)
		{
			log->debug("Creating fullscreen window.");
			window = SDL_CreateWindow("OpenGL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
		}
		else
		{
			log->debug("Creating window with size: {}x{}", width, height);
			window = SDL_CreateWindow("OpenGL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		}

		SDL_ShowCursor(SDL_DISABLE);
		
		context = SDL_GL_CreateContext(window);
		if (context == nullptr)
		{
			sdl_check_result(-1, "Create OpenGL Context");
		}
		int major, minor;
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
		log->debug("Created OpenGL context {}.{}", major, minor);

		// Set vsync for current context 
		set_vsync(vsync);

		if (msaa)
		{
			// Check that MSAA buffers are available
			int msaa_buffers, msaa_samples;
			SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &msaa_buffers);
			SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &msaa_samples);
			msaa_enabled = msaa_buffers > 0 && msaa_samples > 0;
			glEnable(GL_MULTISAMPLE);
		}

		// init glew
		glewExperimental = GL_TRUE;
		auto res = glewInit();
		assert(res == GLEW_OK);
		// ignore GL_INVALID_ENUM after glewInit(), see
		// http://www.opengl.org/wiki/OpenGL_Loading_Library	
		auto error = glGetError();
		assert(error == GL_NO_ERROR || error == GL_INVALID_ENUM);

#ifdef _DEBUG
		gl_check_error();
#endif
	}

	Window::~Window()
	{
		if (context != nullptr)
		{
			SDL_GL_DeleteContext(context);
			context = nullptr;
		}
		if (window != nullptr)
		{
			SDL_DestroyWindow(window);
			window = nullptr;
		}
		SDL_ShowCursor(SDL_ENABLE);
	}

	void Window::resize(int width, int height)
	{
		this->width = width;
		this->height = height;
		SDL_SetWindowSize(window, width, height);
	}

	void Window::set_fullscreen(bool fullscreen)
	{
		this->fullscreen = fullscreen;
		sdl_check_result(SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0),
			"Change screen mode");
		// On Linux, seeing duplicate SDL events get sent after window resize. 
		// Introduce slight delay to avoid this...
		SDL_Delay(100);
	}

	void Window::set_vsync(bool vsync)
	{
		this->vsync = vsync;
#if SDL_VERSION_ATLEAST(1,3,0)
		SDL_GL_SetSwapInterval(vsync);
#else /* SDL_VERSION_ATLEAST(1,3,0) */
#ifdef SDL_GL_SWAP_CONTROL
		SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, vsync);
#else /* SDL_GL_SWAP_CONTROL */
		DEBUG("VSync unsupported on old SDL versions (before 1.2.10).");
#endif /* SDL_GL_SWAP_CONTROL */
#endif /* SDL_VERSION_ATLEAST(1,3,0) */ 
	}

	void Window::on_resize(void)
	{
		SDL_GetWindowSize(window, &width, &height);
		log->debug("Window resized to: {}x{}", width, height);
		trigger(Message{Events::WindowResized, &width, &height});
	}
}