#include "stdafx.h"
#include "log.h"
#include "sysutil.h"
#include "window.h"

namespace dukat
{
	Window::Window(int width, int height, bool fullscreen, bool msaa)
		: width(width), height(height), fullscreen(fullscreen)
	{
		// Need to request MSAA buffers before creating window
		if (msaa) 
		{
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
			SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 1);
		}

		// Create the window
		if (fullscreen)
		{
			logger << "Creating fullscreen window." << std::endl;
			window = SDL_CreateWindow("OpenGL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
		}
		else
		{
			logger << "Creating window with size: " << width << " by " << height << std::endl;
			window = SDL_CreateWindow("OpenGL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
				width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		}

		SDL_ShowCursor(SDL_DISABLE);
		set_vsync(true);

		// Create OpenGL context with desired profile version
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		context = SDL_GL_CreateContext(window);
		if (context == nullptr)
		{
			sdl_check_result(-1, "Create OpenGL Context");
		}
		int major, minor;
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
		logger << "Created OpenGL context " << major << "." << minor << std::endl;

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
		assert(error == GL_INVALID_ENUM);
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

	void Window::set_title(const std::string& title)
	{
		SDL_SetWindowTitle(window, title.c_str());
	}

	void Window::clear(void)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Window::present(void)
	{
		SDL_GL_SwapWindow(window);
	}

	void Window::unbind(WindowEventListener* listener)
	{
		auto it = std::find(event_listeners.begin(), event_listeners.end(), listener);
		if (it != event_listeners.end())
		{
			event_listeners.erase(it);
		}
	}

	void Window::resize(int width, int height)
	{
		this->width = width;
		this->height = height;
		SDL_SetWindowSize(window, width, height);
		on_resize();
	}

	void Window::set_fullscreen(bool fullscreen)
	{
		this->fullscreen = fullscreen;
		sdl_check_result(SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0),
			"Change screen mode");
		on_resize();
	}

	void Window::toggle_fullscreen(void)
	{
		set_fullscreen(!fullscreen);
	}

	void Window::on_resize(void)
	{
		SDL_GetWindowSize(window, &width, &height);
		logger << "New window size: " << width << " * " << height << std::endl;
		// Notify listeners
		for (auto l : event_listeners)
		{
			l->resize(width, height);
		}
	}

	void Window::set_vsync(bool vsync)
	{
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
}