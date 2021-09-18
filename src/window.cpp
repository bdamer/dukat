#include "stdafx.h"
#include <dukat/log.h>
#include <dukat/sdlutil.h>
#include <dukat/sysutil.h>
#include <dukat/window.h>

namespace dukat
{
	Window::Window(const Settings& settings)
	{
		this->width = settings.get_int("window.width", -1);
		this->height = settings.get_int("window.height", -1);
		this->fullscreen = settings.get_bool("window.fullscreen");
		this->resizable = settings.get_bool("window.resizable");

#ifdef OPENGL_CORE
		const auto msaa = settings.get_bool("window.msaa");
		// Need to request MSAA buffers before creating window
		if (msaa)
		{
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 1);
		}
#endif
		set_context_attributes();
		create_window();
		context = SDL_GL_CreateContext(window);
		if (context == nullptr)
			sdl_check_result(-1, "Create OpenGL Context");
		int major, minor;
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
		log->debug("Created OpenGL context {}.{}", major, minor);

		// Set vsync for current context 
		set_vsync(settings.get_bool("window.vsync", true));

#ifdef OPENGL_CORE
		if (msaa)
		{
			// Check that MSAA buffers are available
			int msaa_buffers, msaa_samples;
			SDL_GL_GetAttribute(SDL_GL_MULTISAMPLEBUFFERS, &msaa_buffers);
			SDL_GL_GetAttribute(SDL_GL_MULTISAMPLESAMPLES, &msaa_samples);
			msaa_enabled = msaa_buffers > 0 && msaa_samples > 0;
			glEnable(GL_MULTISAMPLE);
		}
#endif

#ifndef __ANDROID__
		// init glew
		glewExperimental = GL_TRUE;
		auto res = glewInit();
		assert(res == GLEW_OK);
		// ignore GL_INVALID_ENUM after glewInit(), see
		// http://www.opengl.org/wiki/OpenGL_Loading_Library	
		auto error = glGetError();
		assert(error == GL_NO_ERROR || error == GL_INVALID_ENUM);
#endif

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
	}

	void Window::set_context_attributes(void)
	{
		// Create OpenGL context with desired profile version
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, OPENGL_MAJOR_VERSION);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, OPENGL_MINOR_VERSION);
#ifdef OPENGL_CORE
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#elif OPENGL_ES
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif
	}

	void Window::create_window(void)
	{
#ifdef __ANDROID__
		// Ignore provided width & height - reuse existing dimensions
		width = height = -1;
#endif

		if (width == -1 || height == -1)
		{
			SDL_DisplayMode display_mode;
			SDL_GetCurrentDisplayMode(0, &display_mode);
			width = display_mode.w;
			height = display_mode.h;
		}

		log->debug("Creating window with size: {}x{}", width, height);
		Uint32 window_flags;
#ifdef __ANDROID__
		// Ignore provided width & height - reuse existing dimensions
		SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeRight");
		window_flags = SDL_WINDOW_SHOWN;
#else
		window_flags = SDL_WINDOW_OPENGL;
		if (fullscreen)
		{
			window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}
		else
		{
			window_flags |= SDL_WINDOW_OPENGL;
			if (resizable)
				window_flags |= SDL_WINDOW_RESIZABLE;
		}
#endif
		// Create the window with the requested resolution
		window = SDL_CreateWindow(window_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			width, height, window_flags);
		if (window == nullptr)
			sdl_check_result(-1, "Create SDL Window");
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